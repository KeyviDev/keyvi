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
#include <chrono>  //NOLINT
#include <functional>
#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "process.hpp"

#include "dictionary/dictionary_merger.h"
#include "dictionary/dictionary_types.h"
#include "dictionary/fsa/internal/json_value_store.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "index/internal/index_settings.h"
#include "index/internal/segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class MergeJob final {
  struct MergeJobPayload {
    explicit MergeJobPayload(std::vector<segment_t> segments, const boost::filesystem::path& output_filename,
                             const IndexSettings& settings)
        : segments_(segments), output_filename_(output_filename), settings_(settings), process_finished_(false) {}

    MergeJobPayload(MergeJobPayload&&) = default;
    MergeJobPayload& operator=(MergeJobPayload&&) = default;

    std::vector<segment_t> segments_;
    boost::filesystem::path output_filename_;
    const IndexSettings& settings_;
    std::chrono::time_point<std::chrono::system_clock> start_time_;
    std::chrono::time_point<std::chrono::system_clock> end_time_;
    int exit_code_ = -1;
    bool merge_done = false;
    std::atomic_bool process_finished_;
  };

 public:
  // todo: add ability to stop merging for shutdown
  explicit MergeJob(segment_vec_t segments, size_t id, const boost::filesystem::path& output_filename,
                    const IndexSettings& settings)
      : payload_(segments, output_filename, settings), id_(id), external_process_() {}

  ~MergeJob() {
    if (payload_.process_finished_ == false) {
      EndExternalProcess();
    }
  }

  MergeJob(MergeJob&&) = default;
  MergeJob& operator=(MergeJob&&) = default;

  void Run() { DoExternalProcessMerge(); }

  bool TryFinalize() {
    // already finished?
    if (payload_.process_finished_ == true) {
      return true;
    }
    return TryEndExternalProcess();
  }

  void Finalize() {
    if (payload_.process_finished_ == false) {
      EndExternalProcess();
    }
  }

  bool Successful() {
    // todo: handle case when process crashed
    return payload_.exit_code_ == 0;
  }

  const std::vector<segment_t>& Segments() const { return payload_.segments_; }

  const segment_t MergedSegment() const {
    return segment_t(new Segment(payload_.output_filename_, payload_.segments_));
  }

  void SetMerged() { payload_.merge_done = true; }

  const bool Merged() const { return payload_.merge_done; }

  size_t GetId() const { return id_; }

  // todo: ability to kill job/process

 private:
  MergeJobPayload payload_;
  size_t id_;
  std::shared_ptr<TinyProcessLib::Process> external_process_;

  void DoExternalProcessMerge() {
    payload_.start_time_ = std::chrono::system_clock::now();

    std::stringstream command;

    command << payload_.settings_.GetKeyviMergerBin();
    command << " -m 5242880";

    for (auto s : payload_.segments_) {
      command << " -i " << s->GetDictionaryPath().string();
    }

    command << " -o " << payload_.output_filename_.string();
    external_process_.reset(new TinyProcessLib::Process(command.str()));
  }

  bool TryEndExternalProcess() {
    if (external_process_->try_get_exit_status(payload_.exit_code_)) {
      payload_.end_time_ = std::chrono::system_clock::now();
      payload_.process_finished_ = true;
      return true;
    }
    return false;
  }

  void EndExternalProcess() {
    payload_.exit_code_ = external_process_->get_exit_status();
    payload_.end_time_ = std::chrono::system_clock::now();
    payload_.process_finished_ = true;
  }
};

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_MERGE_JOB_H_
