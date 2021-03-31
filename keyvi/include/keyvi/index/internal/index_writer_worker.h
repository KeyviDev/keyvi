//
// keyvi - A key value store.
//
// Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>
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
#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <mutex>  //NOLINT
#include <string>
#include <thread>  //NOLINT
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"

#include "keyvi/dictionary/dictionary_index_compiler.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/index/constants.h"
#include "keyvi/index/internal/index_settings.h"
#include "keyvi/index/internal/merge_job.h"
#include "keyvi/index/internal/merge_policy_selector.h"
#include "keyvi/index/internal/segment.h"
#include "keyvi/index/types.h"
#include "keyvi/util/active_object.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexWriterWorker final {
  using compiler_t = std::shared_ptr<dictionary::JsonDictionaryIndexCompiler>;
  struct IndexPayload {
    explicit IndexPayload(const std::string& index_directory, const keyvi::util::parameters_t& params)
        : compiler_(),
          write_counter_(0),
          segments_(),
          mutex_(),
          index_directory_(index_directory),
          index_toc_file_(index_directory_ / "index.toc"),
          index_toc_file_part_(index_directory_ / "index.toc.part"),
          settings_(params),
          max_concurrent_merges_(settings_.GetMaxConcurrentMerges()),
          max_segments_(settings_.GetMaxSegments()),
          compile_key_threshold_(settings_.GetSegmentCompileKeyThreshold()),
          index_refresh_interval_(settings_.GetRefreshInterval()),
          merge_jobs_(),
          any_delete_(false),
          merge_enabled_(true) {
      segments_ = std::make_shared<segment_vec_t>();
    }

    compiler_t compiler_;
    std::atomic_size_t write_counter_;
    segments_t segments_;
    std::weak_ptr<segment_vec_t> segments_weak_;
    std::mutex mutex_;
    const boost::filesystem::path index_directory_;
    const boost::filesystem::path index_toc_file_;
    const boost::filesystem::path index_toc_file_part_;
    const internal::IndexSettings settings_;
    const size_t max_concurrent_merges_;
    const size_t max_segments_;
    const size_t compile_key_threshold_;
    const size_t index_refresh_interval_;
    std::list<MergeJob> merge_jobs_;
    bool any_delete_;
    std::atomic_bool merge_enabled_;
  };

 public:
  explicit IndexWriterWorker(const std::string& index_directory, const keyvi::util::parameters_t& params)
      : payload_(index_directory, params),
        merge_policy_(merge_policy(keyvi::util::mapGet<std::string>(params, MERGE_POLICY, DEFAULT_MERGE_POLICY))),
        compiler_active_object_(&payload_, std::bind(&index::internal::IndexWriterWorker::ScheduledTask, this),
                                std::chrono::milliseconds(payload_.index_refresh_interval_)) {
    TRACE("construct worker: %s", payload_.index_directory_.c_str());
    LoadIndex();
  }

  IndexWriterWorker& operator=(IndexWriterWorker const&) = delete;
  IndexWriterWorker(const IndexWriterWorker& that) = delete;

  ~IndexWriterWorker() {
    TRACE("destruct worker: %s", payload_.index_directory_.c_str());
    payload_.merge_enabled_ = false;

    // push a function to finish all pending merges
    compiler_active_object_([](IndexPayload& payload) {
      Compile(&payload);
      for (MergeJob& p : payload.merge_jobs_) {
        p.Finalize();
      }
    });
  }

  const_segments_t Segments() {
    segments_t segments = payload_.segments_weak_.lock();
    if (!segments) {
      TRACE("recreate segments weak ptr");
      std::unique_lock<std::mutex> lock(payload_.mutex_);
      payload_.segments_weak_ = payload_.segments_;
      segments = payload_.segments_;
    }
    return segments;
  }

  // todo: rvalue version??
  void Add(const std::string& key, const std::string& value) {
    // push function
    TRACE("add key %s, pt: %p", key.c_str(), &key);

    // strings are copied
    compiler_active_object_([key, value](IndexPayload& payload) {
      CreateCompilerIfNeeded(&payload);
      TRACE("add_async key %s, pt: %p", key.c_str(), &key);
      payload.compiler_->Add(key, value);
    });

    CompileIfThresholdIsHit();
  }

  template <typename ContainerType>
  void Add(const std::shared_ptr<ContainerType>& key_values) {
    TRACE("bulk add keys: %ul", key_values->size());

    // the shared pointer is copied (not the key/values)
    compiler_active_object_([key_values](IndexPayload& payload) {
      CreateCompilerIfNeeded(&payload);

      for (auto key_value : *key_values) {
        TRACE("add_async key %s, pt: %p", key_value.first.c_str(), &key_value.first);
        payload.compiler_->Add(key_value.first, key_value.second);
      }
    });
    CompileIfThresholdIsHit();
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

    CompileIfThresholdIsHit();
  }

  /**
   * Flush for external use.
   */
  void Flush(const bool async = false) {
    TRACE("flush");

    if (async) {
      compiler_active_object_([](IndexPayload& payload) {
        PersistDeletes(&payload);
        Compile(&payload);
      });
    } else {
      std::mutex m;
      std::condition_variable c;
      std::unique_lock<std::mutex> lock(m);

      compiler_active_object_([&m, &c](IndexPayload& payload) {
        PersistDeletes(&payload);
        Compile(&payload);
        std::unique_lock<std::mutex> waitLock(m);
        c.notify_all();
      });

      c.wait(lock);
    }
  }

  void ForceMerge(const size_t max_segments) {
    TRACE("force merge");

    // 1st check the queue and empty it if necessary
    if (compiler_active_object_.Size() > 0) {
      Flush();
    }

    // spin until we reach the desired size
    while (payload_.segments_->size() > max_segments) {
      // wait some time for segments being merged
      // todo improve this dependent on number and size of segments
      std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_FOR_SEGMENT_MERGES_MS));

      // should we somehow got new data, flush again
      if (compiler_active_object_.Size() > 0) {
        Flush();
      }
    }
  }

 private:
  IndexPayload payload_;
  merge_policy_t merge_policy_;
  util::ActiveObject<IndexPayload> compiler_active_object_;

  void CompileIfThresholdIsHit() {
    if (++payload_.write_counter_ > payload_.compile_key_threshold_) {
      compiler_active_object_([](IndexPayload& payload) { Compile(&payload); });
      payload_.write_counter_ = 0;

      // worst case scenario, to many segments, throttle further writes until we are below the limit
      while (compiler_active_object_.Size() + payload_.segments_->size() >= payload_.max_segments_) {
        // wait some time and then flush, which should give time to reduce the number of open file descriptors
        std::this_thread::sleep_for(std::chrono::milliseconds(SPINLOCK_WAIT_FOR_SEGMENT_MERGES_MS));
        Flush();
      }
    }
  }

  void ScheduledTask() {
    TRACE("Scheduled task");

    if (payload_.merge_jobs_.size()) {
      FinalizeMerge();
    }

    if (payload_.merge_enabled_) {
      RunMerge();
    }

    if (!payload_.compiler_ && !payload_.any_delete_) {
      return;
    }

    PersistDeletes(&payload_);
    Compile(&payload_);
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

          // thread-safe swap
          {
            std::unique_lock<std::mutex> lock(payload_.mutex_);
            payload_.segments_.swap(new_segments);
          }
          WriteToc(&payload_);

          // reset as segments have been changed
          payload_.segments_weak_.reset();

          // delete old segment files
          for (const segment_t& s : p.Segments()) {
            TRACE("delete old file: %s", s->GetDictionaryFilename().c_str());

            // if the segment is somehow used, ensure file handles have access to it
            // this should be safe because we swapped the old segments out and reseted the weak ptr
            if (s.use_count() > 1) {
              s->Load();
            }
            // if there are open file handles the OS defers real deletion for us
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
    if (payload_.merge_jobs_.size() == payload_.max_concurrent_merges_) {
      // to many merges already running, so throttle
      return;
    }

    size_t merge_policy_id = 0;
    std::vector<segment_t> to_merge;

    if (merge_policy_->SelectMergeSegments(payload_.segments_, &to_merge, &merge_policy_id) == false) {
      return;
    }

    TRACE("enough segments found for merging");
    boost::filesystem::path p(payload_.index_directory_);
    p /= boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.kv");

    for (segment_t& s : to_merge) {
      s->ElectedForMerge();
    }

    payload_.merge_jobs_.emplace_back(to_merge, merge_policy_id, p, payload_.settings_);

    // force external merge if low on filedescriptors
    payload_.merge_jobs_.back().Run(payload_.segments_->size() + to_merge.size() + 10 > payload_.max_segments_);
  }

  void LoadIndex() {
    std::ifstream toc_fstream(payload_.index_toc_file_.string());

    if (!toc_fstream.good()) {
      // empty index
      return;
    }

    rapidjson::Document index_toc;
    rapidjson::IStreamWrapper isw(toc_fstream);
    index_toc.ParseStream(isw);

    TRACE("index_toc loaded");

    TRACE("reading segments");

    for (const auto& e : index_toc["files"].GetArray()) {
      boost::filesystem::path p(payload_.index_directory_);
      p /= e.GetString();
      payload_.segments_->emplace_back(new Segment(p, false));
    }
  }

  static inline void PersistDeletes(IndexPayload* payload) {
    // only loop through segments if any delete has happened
    if (payload->any_delete_) {
      for (segment_t& s : *payload->segments_) {
        if (s->Persist()) {
          s->ReloadDeletedKeys();
        }
      }
    }

    // clear delete flag
    payload->any_delete_ = false;
  }

  static inline void CreateCompilerIfNeeded(IndexPayload* payload) {
    if (!payload->compiler_) {
      TRACE("recreate compiler");
      keyvi::util::parameters_t params = keyvi::util::parameters_t{{"memory_limit_mb", "5"}};

      payload->compiler_.reset(new dictionary::JsonDictionaryIndexCompiler(params));
    }
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

    // add/register new segment
    // we have to copy the segments (shallow copy/list of shared pointers to segments)
    // and then swap it
    segment_t new_segment(new Segment(p, true));
    segments_t new_segments = std::make_shared<segment_vec_t>(*payload->segments_);
    new_segments->push_back(new_segment);

    // thread-safe swap
    {
      std::unique_lock<std::mutex> lock(payload->mutex_);
      payload->segments_.swap(new_segments);
    }

    WriteToc(payload);

    // reset as segments have been changed
    payload->segments_weak_.reset();
  }

  static void WriteToc(const IndexPayload* payload) {
    TRACE("write new TOC");

    std::ofstream out_stream(payload->index_toc_file_part_.string());
    {
      rapidjson::OStreamWrapper ostream_wrapper(out_stream);
      rapidjson::Writer<rapidjson::OStreamWrapper> writer(ostream_wrapper);

      TRACE("Number of segments: %ld", payload->segments_->size());

      writer.StartObject();
      writer.Key("files");
      writer.StartArray();
      for (const auto& s : *(payload->segments_)) {
        TRACE("put %s", s->GetDictionaryFilename().c_str());
        writer.String(s->GetDictionaryFilename());
      }

      writer.EndArray();
      writer.EndObject();
    }
    boost::filesystem::rename(payload->index_toc_file_part_, payload->index_toc_file_);
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_WRITER_WORKER_H_
