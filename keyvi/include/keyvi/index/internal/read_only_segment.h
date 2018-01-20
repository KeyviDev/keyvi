/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * read_only_segment.h
 *
 *  Created on: Jan 20, 2018
 *      Author: hendrik
 */

#ifndef KEYVI_INDEX_INTERNAL_READ_ONLY_SEGMENT_H_
#define KEYVI_INDEX_INTERNAL_READ_ONLY_SEGMENT_H_

#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "dictionary/dictionary.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class ReadOnlySegment {
 public:
  explicit ReadOnlySegment(const boost::filesystem::path& path, const bool load = true)
      : path_(path), filename_(path.filename().string()), dictionary_(), deleted_keys_() {
    if (load) {
      Load();
    }
  }

  dictionary::dictionary_t& operator*() {
    if (!dictionary_) {
      Load();
    }

    return dictionary_;
  }

  dictionary::dictionary_t& GetDictionary() {
    if (!dictionary_) {
      Load();
    }

    return dictionary_;
  }

  const boost::filesystem::path& GetPath() const { return path_; }

  const std::string& GetFilename() const { return filename_; }

 protected:
  void Load() {
    dictionary_.reset(new dictionary::Dictionary(path_.string()));
    // load deleted keys
  }

 private:
  boost::filesystem::path path_;
  std::string filename_;
  dictionary::dictionary_t dictionary_;
  std::unordered_set<std::string> deleted_keys_;
};

typedef std::shared_ptr<ReadOnlySegment> read_only_segment_t;
typedef std::vector<read_only_segment_t> read_only_segment_vec_t;
typedef std::shared_ptr<read_only_segment_vec_t> read_only_segments_t;
typedef const std::shared_ptr<read_only_segment_vec_t> const_read_only_segments_t;

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_READ_ONLY_SEGMENT_H_
