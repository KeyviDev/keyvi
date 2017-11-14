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
 * index_finalizer.h
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
#include <list>
#include <memory>
#include <string>
#include <thread>  //NOLINT
#include <vector>

#include "dictionary/dictionary_compiler.h"
#include "dictionary/dictionary_types.h"
#include "index/internal/merge_job.h"
#include "index/internal/segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class IndexWriterWorker final {
  typedef std::function<void(const std::string&)> finalizer_callback_t;
  typedef dictionary::JsonDictionaryCompilerSmallData compiler_t;
  typedef std::shared_ptr<compiler_t> compiler_t_ptr;

 public:
  explicit IndexWriterWorker(const std::string& index_directory,
                             const std::chrono::duration<double>& flush_interval = std::chrono::milliseconds(1000))
      : compiler_(),
        compiler_to_flush_(),
        do_flush_(false),
        index_mutex_(),
        flush_cond_mutex_(),
        flush_cond_(),
        finalizer_thread_(),
        stop_finalizer_thread_(true),
        write_counter_(0),
        segments_(),
        index_directory_(index_directory),
        merge_jobs_(),
        last_flush_(),
        flush_interval_(flush_interval) {}

  void StartWorkerThread() {
    if (stop_finalizer_thread_ == false) {
      // already runs
      return;
    }

    stop_finalizer_thread_ = false;
    TRACE("Start Finalizer thread");
    finalizer_thread_ = std::thread(&IndexWriterWorker::Finalizer, this);
  }

  void StopWorkerThread() {
    stop_finalizer_thread_ = true;

    // todo: joinable blocks
    if (finalizer_thread_.joinable()) {
      finalizer_thread_.join();
      TRACE("worker thread joined");
      TRACE("Open merges: %ld", merge_jobs_.size());
    }

    for (MergeJob& p : merge_jobs_) {
      p.Wait();
    }
    FinalizeMerge();
  }

  compiler_t* AcquireCompiler() {
    compiler_in_use_ = true;

    compiler_t* c = compiler_.get();

    if (c == nullptr) {
      TRACE("re-create compiler");
      dictionary::compiler_param_t params = dictionary::compiler_param_t{{STABLE_INSERTS, "true"}, {MEMORY_LIMIT_KEY, "5242880"}};

      compiler_.reset(new compiler_t(params));
      c = compiler_.get();
    }

    return c;
  }

  void ReleaseCompiler() {
    // while the current compiler was in use, check whether the
    // background worker tried to flush
    if (compiler_bg_dirty_ && compiler_to_flush_bg_.get() != nullptr) {
      CompileAndRegister(&compiler_to_flush_bg_);
    }

    if (++write_counter_ > 1000) {
      if (DoFlush()) {
        write_counter_ = 0;
        // todo: throttle needed if the background compile cannot catch up
      } else if (write_counter_ > 10000) {
        std::this_thread::yield();
      }
    }

    compiler_in_use_ = false;
  }

  const std::vector<segment_t>& Segments() const { return segments_; }

  /**
   * Flush for external use.
   */
  void Flush(bool async = true) {
    // ensure that background flush does not kick in
    compiler_in_use_ = true;
    if (compiler_bg_dirty_ && compiler_to_flush_bg_.get() != nullptr) {
      CompileAndRegister(&compiler_to_flush_bg_);
    }

    DoFlush(async);
    compiler_in_use_ = false;
  }

 private:
  compiler_t_ptr compiler_;
  compiler_t_ptr compiler_to_flush_;
  compiler_t_ptr compiler_to_flush_bg_;

  std::atomic_bool compiler_in_use_;
  std::atomic_bool compiler_bg_dirty_;

  std::atomic_bool do_flush_;
  std::recursive_mutex index_mutex_;
  std::mutex flush_cond_mutex_;
  std::condition_variable flush_cond_;
  std::thread finalizer_thread_;
  std::atomic_bool stop_finalizer_thread_;
  std::atomic_size_t write_counter_;
  std::vector<segment_t> segments_;
  boost::filesystem::path index_directory_;
  std::list<MergeJob> merge_jobs_;
  std::chrono::system_clock::time_point last_flush_;
  std::chrono::duration<double> flush_interval_;
  std::chrono::duration<double> finalizer_poll_interval_ = std::chrono::milliseconds(10);
  size_t max_concurrent_merges = 2;

  bool DoFlush(bool async = true) {
    if (do_flush_ || compiler_.get() == nullptr) {
      return false;
    }

    // no need for making it atomic??
    compiler_.swap(compiler_to_flush_);
    // std::atomic_store(&compiler_, compiler_to_flush_);

    if (async) {
      do_flush_ = true;
      flush_cond_.notify_one();
      return true;
    }  //  else

    // TODO: blocking implementation
    return true;
  }

  void Finalizer() {
    std::unique_lock<std::mutex> l(flush_cond_mutex_);
    TRACE("Finalizer loop");
    while (!stop_finalizer_thread_) {
      TRACE("Finalizer, check for finalization.");
      FinalizeMerge();
      RunMerge();

      // sleep for some time or until woken up
      flush_cond_.wait_for(l, finalizer_poll_interval_);
      auto tp = std::chrono::system_clock::now();
      TRACE("wakeup finalizer %s %ld ***", l.owns_lock() ? "true" : "false", tp.time_since_epoch());

      if (do_flush_ == true || tp - last_flush_ > flush_interval_) {
        Compile();
        do_flush_ = false;

        last_flush_ = tp;
      }
    }

    // check that there are no open compilers
    if (compiler_to_flush_bg_.get() != nullptr) {
      CompileAndRegister(&compiler_to_flush_bg_);
    }

    if (compiler_to_flush_.get() != nullptr) {
      CompileAndRegister(&compiler_to_flush_);
    }

    if (compiler_.get() != nullptr) {
      CompileAndRegister(&compiler_);
    }

    TRACE("Finalizer loop stop");
  }

  void Compile() {
    TRACE("compile");
    // [1] check if there is already a compiler waiting to get finalized
    // while there is no compiler running at the moment
    if (compiler_in_use_ == false && compiler_to_flush_bg_.get() != nullptr) {
      compiler_bg_dirty_ = false;
      CompileAndRegister(&compiler_to_flush_bg_);
    }

    if (compiler_to_flush_.get() == nullptr) {
      // nothing to flush, check if triggered by flush interval
      if (do_flush_ == false) {
        // check if there is something to compile
        if (compiler_.get() != nullptr) {
          // get the state  before swapping
          bool compiler_in_use = compiler_in_use_;
          compiler_bg_dirty_ = false;

          // not atomic, mitigated by the atomic_bool
          compiler_to_flush_bg_.swap(compiler_);

          // reset the counter
          write_counter_ = 0;

          // it's possible that the compiler got null while  swapping
          if (compiler_to_flush_bg_.get() == nullptr) {
            return;
          }

          // note: compiler might be in use
          // in the low likely event that while swapping a compiler was
          // returned and in_use changed to false, compile will finish at [1],
          // at the next flush
          if (compiler_in_use) {
            compiler_bg_dirty_ = true;
          } else {
            CompileAndRegister(&compiler_to_flush_bg_);
          }
        }
      }
      return;
    }
    CompileAndRegister(&compiler_to_flush_);
  }

  void CompileAndRegister(compiler_t_ptr* compiler) {
    compiler->get()->Compile();

    boost::filesystem::path p(index_directory_);
    p /= boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.kv");

    TRACE("write to file %s %s", p.string().c_str(), p.filename().string().c_str());

    compiler->get()->WriteToFile(p.string());

    // free up resources
    compiler->reset();
    segment_t w(new Segment(p));
    // register segment
    RegisterSegment(w);

    TRACE("Segment compiled and registered");
  }

  /**
   * Check if any merge process is done and finalize if necessary
   */
  void FinalizeMerge() {
    bool any_merge_finalized = false;

    TRACE("Finalize Merge");
    for (MergeJob& p : merge_jobs_) {
      if (p.TryFinalize()) {
        if (p.Successful()) {
          TRACE("rewriting segment list");
          any_merge_finalized = true;

          std::lock_guard<std::recursive_mutex> lock(index_mutex_);

          // remove old segments and replace it with new one
          std::vector<segment_t> new_segments;
          bool merged_new_segment = false;

          std::copy_if(segments_.begin(), segments_.end(), std::back_inserter(new_segments),
                       [&new_segments, &merged_new_segment, &p](const segment_t& s) {

                         TRACE("checking %s", s->GetFilename().c_str());
                         if (std::count_if(p.Segments().begin(), p.Segments().end(), [s](const segment_t& s2) {
                               return s2->GetFilename() == s->GetFilename();
                             })) {
                           if (!merged_new_segment) {
                             new_segments.push_back(p.MergedSegment());
                             merged_new_segment = true;
                           }
                           return false;
                         }
                         return true;
                       });
          TRACE("merged segment %s", p.MergedSegment()->GetFilename().c_str());
          TRACE("1st segment after merge: %s", new_segments[0]->GetFilename().c_str());

          segments_.swap(new_segments);
          WriteToc();

          // delete old segment files
          for (const segment_t& s : p.Segments()) {
            TRACE("delete old file: %s", s->GetFilename().c_str());
            std::remove(s->GetPath().string().c_str());
          }

          p.SetMerged();

        } else {
          // the merge process failed
          TRACE("merge failed, reset markers");
          // mark all segments as mergeable again
          for (const segment_t& s : p.Segments()) {
            s->UnMarkMerge();
          }

          // todo throttle strategy?
        }
      }
      // else the merge is still running, maybe check how long it already runs
    }

    if (any_merge_finalized) {
      TRACE("delete merge job");

      merge_jobs_.remove_if([](const MergeJob& j) { return j.Merged(); });
    }
  }

  /**
   * Run a merge if mergers are available and segments require merge
   */
  void RunMerge() {
    // to few segments, return
    if (segments_.size() <= 1) {
      return;
    }

    if (merge_jobs_.size() == max_concurrent_merges) {
      // to many merges already running, so throttle
      return;
    }

    std::vector<segment_t> to_merge;
    for (segment_t& s : segments_) {
      if (!s->MarkedForMerge()) {
        TRACE("Add to merge list %s", s->GetFilename().c_str());
        to_merge.push_back(s);
      }
    }

    if (to_merge.size() < 1) {
      return;
    }

    TRACE("enough segments found for merging");
    boost::filesystem::path p(index_directory_);
    p /= boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.kv");

    for (segment_t& s : to_merge) {
      s->MarkMerge();
    }

    // todo: add id
    merge_jobs_.emplace_back(to_merge, 0, p);
    merge_jobs_.back().Run();
  }

  void RegisterSegment(segment_t segment) {
    std::lock_guard<std::recursive_mutex> lock(index_mutex_);
    TRACE("add segment %s", segment->GetFilename().c_str());
    segments_.push_back(segment);
    WriteToc();
  }

  void WriteToc() {
    std::lock_guard<std::recursive_mutex> lock(index_mutex_);
    TRACE("write new TOC");

    boost::property_tree::ptree ptree;
    boost::property_tree::ptree files;

    TRACE("Number of segments: %ld", segments_.size());

    for (auto s : segments_) {
      TRACE("put %s", s->GetFilename().c_str());
      boost::property_tree::ptree sp;
      sp.put("", s->GetFilename());
      files.push_back(std::make_pair("", sp));
    }

    ptree.add_child("files", files);
    boost::filesystem::path p(index_directory_);
    p /= "index.toc.part";

    boost::filesystem::path p2(index_directory_);
    p2 /= "index.toc";

    boost::property_tree::write_json(p.string(), ptree);
    boost::filesystem::rename(p, p2);
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_INDEX_WRITER_WORKER_H_
