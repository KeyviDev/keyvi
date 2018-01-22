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
#include <ctime>
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
      : dictionary_path_(path),
        deleted_keys_path_(path),
        deleted_keys_during_merge_path_(path),
        dictionary_filename_(path.filename().string()),
        dictionary_(),
        has_deleted_keys_(false),
        deleted_keys_(),
        last_modification_time_deleted_keys_(0),
        last_modification_time_deleted_keys_during_merge_(0) {
    deleted_keys_path_ += ".dk";
    deleted_keys_during_merge_path_ += ".dkm";
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

  void ReloadDeletedKeys() {}

  const boost::filesystem::path& GetDictionaryPath() const { return dictionary_path_; }

  const boost::filesystem::path& GetDeletedKeysPath() const { return deleted_keys_path_; }

  const boost::filesystem::path& GetDeletedKeysDuringMergePath() const { return deleted_keys_during_merge_path_; }

  const std::string& GetDictionaryFilename() const { return dictionary_filename_; }

  bool HasDeletedKeys() const { return has_deleted_keys_; }

 protected:
  void Load() {
    // load dictionary
    dictionary_.reset(new dictionary::Dictionary(dictionary_path_.string()));

    // load deleted keys
    LoadDeletedKeys();
  }

 private:
  //! path of the underlying dictionary
  boost::filesystem::path dictionary_path_;

  //! list of deleted keys
  boost::filesystem::path deleted_keys_path_;

  //! deleted keys while segment gets merged with other segments
  boost::filesystem::path deleted_keys_during_merge_path_;

  //! just the filename part of the dictionary
  std::string dictionary_filename_;

  //! the dictionary itself
  dictionary::dictionary_t dictionary_;

  //! quick and cheap check whether this segment has deletes (assuming that deletes are rare)
  std::atomic_bool has_deleted_keys_;

  //! deleted keys
  std::shared_ptr<std::unordered_set<std::string>> deleted_keys_;

  //! last modification time for the deleted keys file
  std::time_t last_modification_time_deleted_keys_;

  //! last modification time for the deleted keys file during a merge operation
  std::time_t last_modification_time_deleted_keys_during_merge_;

  void LoadDeletedKeys() {}
};

typedef std::shared_ptr<ReadOnlySegment> read_only_segment_t;
typedef std::vector<read_only_segment_t> read_only_segment_vec_t;
typedef std::shared_ptr<read_only_segment_vec_t> read_only_segments_t;
typedef const std::shared_ptr<read_only_segment_vec_t> const_read_only_segments_t;

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_READ_ONLY_SEGMENT_H_
