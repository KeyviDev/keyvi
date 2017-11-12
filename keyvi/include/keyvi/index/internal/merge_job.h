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
#ifndef KEYVI_INDEX_INTERNAL_MERGE_JOB_H_
#define KEYVI_INDEX_INTERNAL_MERGE_JOB_H_

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "process.hpp"

#include "dictionary/dictionary_merger.h"
#include "dictionary/dictionary_types.h"
#include "dictionary/fsa/internal/json_value_store.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "index/internal/segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class MergeJob final {
  struct MergeJobPayload {
    explicit MergeJobPayload(std::vector<segment_t> segments, const boost::filesystem::path& output_filename)
        : segments_(segments), output_filename_(output_filename), process_finished_(false) {}

    MergeJobPayload(MergeJobPayload&&) = default;
    MergeJobPayload& operator=(MergeJobPayload&&) = default;

    std::vector<segment_t> segments_;
    boost::filesystem::path output_filename_;
    std::chrono::time_point<std::chrono::system_clock> start_time_;
    std::chrono::time_point<std::chrono::system_clock> end_time_;
    int exit_code_ = -1;
    bool merge_done = false;
    std::atomic_bool process_finished_;
  };

 public:
  // todo: add ability to stop merging for shutdown
  explicit MergeJob(std::vector<segment_t> segments, size_t id, const boost::filesystem::path& output_filename)
      : payload_(segments, output_filename), id_(id), job_thread_() {}

  MergeJob(MergeJob&&) = default;
  MergeJob& operator=(MergeJob&&) = default;

  void Run() {
    MergeJobPayload* job = &payload_;
    job_thread_ = std::thread([job]() {
      job->start_time_ = std::chrono::system_clock::now();

      TinyProcessLib::Process merge_process([job] {
        try {
          dictionary::DictionaryMerger<dictionary::fsa::internal::SparseArrayPersistence<>,
                                       dictionary::fsa::internal::JsonValueStore>
              m(dictionary::merger_param_t({{"memory_limit_mb", "10"}}));

          for (auto s : job->segments_) {
            m.Add(s->GetPath().string());
          }

          TRACE("merge done");
          m.Merge(job->output_filename_.string());
          exit(0);
        } catch (std::exception& e) {
          exit(1);
        }
      });

      job->exit_code_ = merge_process.get_exit_status();
      job->end_time_ = std::chrono::system_clock::now();
      job->process_finished_ = true;
      TRACE("Merge finished with %ld", job->exit_code_);
    });
  }

  bool isRunning() const {
    TRACE("Process finished %s", payload_.process_finished_ ? "yes" : "no");
    return !payload_.process_finished_;
  }

  bool TryFinalize() {
    // already joined
    if (!job_thread_.joinable()) {
      return true;
    }

    if (payload_.process_finished_) {
      job_thread_.join();
      return true;
    }

    return false;
  }

  bool Successful() {
    // todo: handle case when process crashed
    return payload_.exit_code_ == 0;
  }

  const std::vector<segment_t>& Segments() const { return payload_.segments_; }

  const segment_t MergedSegment() const { return segment_t(new Segment(payload_.output_filename_, false)); }

  void SetMerged() { payload_.merge_done = true; }

  const bool Merged() const { return payload_.merge_done; }

  void Wait() { job_thread_.join(); }

  // todo: ability to kill job/process

 private:
  MergeJobPayload payload_;
  size_t id_;
  std::thread job_thread_;
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_MERGE_JOB_H_
