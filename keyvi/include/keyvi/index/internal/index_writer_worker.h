//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * index_writer_worker.h
 *
 *  Created on: Jan 18, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_INDEX_WRITER_WORKER_H_
#define KEYVI_INDEX_INTERNAL_INDEX_WRITER_WORKER_H_

#include <algorithm>
#include <atomic>
#include <condition_variable>  //NOLINT
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <mutex>  //NOLINT
#include <string>
#include <thread>  //NOLINT
#include <vector>

#include "dictionary/dictionary_compiler.h"
#include "dictionary/dictionary_types.h"
#include "index/internal/constants.h"
#include "index/internal/merge_job.h"
#include "index/internal/merge_policy_selector.h"
#include "index/internal/segment.h"
#include "util/active_object.h"
#include "util/configuration.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexWriterWorker final {
  typedef std::shared_ptr<dictionary::JsonDictionaryCompilerSmallData> compiler_t;
  struct IndexPayload {
    explicit IndexPayload(const std::string& index_directory, const std::chrono::milliseconds& refresh_interval)
        : compiler_(),
          write_counter_(0),
          segments_(),
          index_directory_(index_directory),
          merge_jobs_(),
          last_flush_(),
          refresh_interval_(refresh_interval),
          any_delete_(false),
          merge_enabled_(true) {
      segments_ = std::make_shared<segment_vec_t>();
    }

    compiler_t compiler_;
    std::atomic_size_t write_counter_;
    segments_t segments_;
    boost::filesystem::path index_directory_;
    std::list<MergeJob> merge_jobs_;
    std::chrono::system_clock::time_point last_flush_;
    std::chrono::milliseconds refresh_interval_;
    size_t max_concurrent_merges_ = 2;
    bool any_delete_;
    std::atomic_bool merge_enabled_;
  };

 public:
  explicit IndexWriterWorker(const std::string& index_directory, const keyvi::util::parameters_t& params)
      : payload_(index_directory,
                 std::chrono::milliseconds(keyvi::util::mapGet<uint64_t>(params, INDEX_REFRESH_INTERVAL, 1000))),
        compiler_active_object_(
            &payload_, std::bind(&index::internal::IndexWriterWorker::ScheduledTask, this),
            std::chrono::milliseconds(keyvi::util::mapGet<uint64_t>(params, INDEX_REFRESH_INTERVAL, 1000))) {
    TRACE("construct worker: %s", payload_.index_directory_.c_str());

    merge_policy_.reset(merge_policy(keyvi::util::mapGet<std::string>(params, MERGE_POLICY, DEFAULT_MERGE_POLICY)));
  }

  IndexWriterWorker& operator=(IndexWriterWorker const&) = delete;
  IndexWriterWorker(const IndexWriterWorker& that) = delete;

  ~IndexWriterWorker() {
    TRACE("destruct worker: %s", payload_.index_directory_.c_str());
    payload_.merge_enabled_ = false;

    // push a function to finish all pending merges
    compiler_active_object_([](IndexPayload& payload) {
      for (MergeJob& p : payload.merge_jobs_) {
        p.Finalize();
      }
    });
  }

  const_segments_t Segments() const { return payload_.segments_; }

  // todo: rvalue version??
  void Add(const std::string& key, const std::string& value) {
    // push function
    compiler_active_object_([key, value](IndexPayload& payload) {
      // todo non-lazy?
      if (!payload.compiler_) {
        TRACE("recreate compiler");
        keyvi::util::parameters_t params =
            keyvi::util::parameters_t{{STABLE_INSERTS, "true"}, {"memory_limit_mb", "5"}};

        payload.compiler_.reset(new dictionary::JsonDictionaryCompilerSmallData(params));
      }
      TRACE("add key %s", key.c_str());
      payload.compiler_->Add(key, value);
    });

    if (++payload_.write_counter_ > 1000) {
      compiler_active_object_([](IndexPayload& payload) { Compile(&payload); });
      payload_.write_counter_ = 0;
    }
  }

  void Delete(const std::string& key) {
    compiler_active_object_([key](IndexPayload& payload) {
      payload.any_delete_ = true;
      TRACE("delete key %s", key.c_str());

      if (payload.compiler_) {
        payload.compiler_->Delete(key);
      }

      if (payload.segments_) {
        for (const segment_t& s : *payload.segments_) {
          s->DeleteKey(key);
        }
      }
    });

    if (++payload_.write_counter_ > 1000) {
      compiler_active_object_([](IndexPayload& payload) { Compile(&payload); });
      payload_.write_counter_ = 0;
    }
  }

  /**
   * Flush for external use.
   */
  void Flush(bool async = true) {
    TRACE("flush");

    compiler_active_object_([](IndexPayload& payload) {
      PersistDeletes(&payload);
      Compile(&payload);
    });

    if (async == false) {
      std::mutex m;
      std::condition_variable c;
      std::unique_lock<std::mutex> lock(m);

      compiler_active_object_([&m, &c](IndexPayload& payload) {
        std::unique_lock<std::mutex> waitLock(m);
        c.notify_all();
      });

      c.wait(lock);
    }
  }

 private:
  IndexPayload payload_;
  std::unique_ptr<MergePolicy> merge_policy_;
  util::ActiveObject<IndexPayload> compiler_active_object_;

  void ScheduledTask() {
    TRACE("Scheduled task");
    FinalizeMerge();
    if (payload_.merge_enabled_) {
      RunMerge();
    }

    if (!payload_.compiler_ && !payload_.any_delete_) {
      return;
    }

    auto tp = std::chrono::system_clock::now();
    if (tp - payload_.last_flush_ > payload_.refresh_interval_) {
      PersistDeletes(&payload_);
      Compile(&payload_);
      payload_.last_flush_ = tp;
    }
  }

  /**
   * Check if any merge process is done and finalize if necessary
   */
  void FinalizeMerge() {
    bool any_merge_finalized = false;

    TRACE("Finalize Merge");
    for (MergeJob& p : payload_.merge_jobs_) {
      if (p.TryFinalize()) {
        if (p.Successful()) {
          // let the merge policy know that id is done
          merge_policy_->MergeFinished(p.GetId());

          TRACE("rewriting segment list");
          any_merge_finalized = true;

          // remove old segments and replace it with new one
          segments_t new_segments = std::make_shared<segment_vec_t>();

          bool merged_new_segment = false;

          std::copy_if(payload_.segments_->begin(), payload_.segments_->end(), std::back_inserter(*new_segments),
                       [&new_segments, &merged_new_segment, &p](const segment_t& s) {
                         TRACE("checking %s", s->GetDictionaryFilename().c_str());
                         if (std::count_if(p.Segments().begin(), p.Segments().end(), [s](const segment_t& s2) {
                               return s2->GetDictionaryFilename() == s->GetDictionaryFilename();
                             })) {
                           if (!merged_new_segment) {
                             new_segments->push_back(p.MergedSegment());
                             merged_new_segment = true;
                           }
                           return false;
                         }
                         return true;
                       });
          TRACE("merged segment %s", p.MergedSegment()->GetDictionaryFilename().c_str());
          TRACE("1st segment after merge: %s", (*new_segments)[0]->GetDictionaryFilename().c_str());

          // todo: thread-safe?
          payload_.segments_ = new_segments;
          WriteToc(&payload_);

          // delete old segment files
          for (const segment_t& s : p.Segments()) {
            TRACE("delete old file: %s", s->GetDictionaryFilename().c_str());
            s->RemoveFiles();
          }

          p.SetMerged();

        } else {
          // the merge process failed
          TRACE("merge failed, reset markers");
          // mark all segments as mergeable again
          for (const segment_t& s : p.Segments()) {
            s->MergeFailed();
          }

          // todo throttle strategy?
        }
      }
      // else the merge is still running, maybe check how long it already runs
    }

    if (any_merge_finalized) {
      TRACE("delete merge job");

      payload_.merge_jobs_.remove_if([](const MergeJob& j) { return j.Merged(); });
    }
  }

  /**
   * Run a merge if mergers are available and segments require merge
   */
  void RunMerge() {
    // to few segments, return
    if (payload_.segments_->size() <= 1) {
      return;
    }

    if (payload_.merge_jobs_.size() == payload_.max_concurrent_merges_) {
      // to many merges already running, so throttle
      return;
    }

    size_t merge_policy_id = 0;
    std::vector<segment_t> to_merge = merge_policy_->SelectMergeSegments(payload_.segments_, &merge_policy_id);

    if (to_merge.size() < 1) {
      return;
    }

    TRACE("enough segments found for merging");
    boost::filesystem::path p(payload_.index_directory_);
    p /= boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.kv");

    for (segment_t& s : to_merge) {
      s->ElectedForMerge();
    }

    payload_.merge_jobs_.emplace_back(to_merge, merge_policy_id, p);
    payload_.merge_jobs_.back().Run();
  }

  static inline void PersistDeletes(IndexPayload* payload) {
    // only loop through segments if any delete has happened
    if (payload->any_delete_) {
      for (segment_t& s : *payload->segments_) {
        s->Persist();
      }
    }

    // clear delete flag
    payload->any_delete_ = false;
  }

  static inline void Compile(IndexPayload* payload) {
    if (!payload->compiler_) {
      TRACE("no compiler found");
      return;
    }

    boost::filesystem::path p(payload->index_directory_);
    p /= boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.kv");

    TRACE("compiling");
    payload->compiler_->Compile();
    TRACE("write to file [%s] [%s]", p.string().c_str(), p.filename().string().c_str());

    payload->compiler_->WriteToFile(p.string());

    // free resources
    payload->compiler_.reset();

    segment_t w(new Segment(p));
    // register segment
    payload->segments_->push_back(w);
    WriteToc(payload);
  }

  static void WriteToc(const IndexPayload* payload) {
    TRACE("write new TOC");

    boost::property_tree::ptree ptree;
    boost::property_tree::ptree files;

    TRACE("Number of segments: %ld", payload->segments_->size());

    for (const auto s : *(payload->segments_)) {
      TRACE("put %s", s->GetDictionaryFilename().c_str());
      boost::property_tree::ptree sp;
      sp.put("", s->GetDictionaryFilename());
      files.push_back(std::make_pair("", sp));
    }

    ptree.add_child("files", files);
    boost::filesystem::path p(payload->index_directory_);
    p /= "index.toc.part";

    boost::filesystem::path p2(payload->index_directory_);
    p2 /= "index.toc";

    boost::property_tree::write_json(p.string(), ptree);
    boost::filesystem::rename(p, p2);
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_WRITER_WORKER_H_
