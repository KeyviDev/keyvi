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

#ifndef KEYVI_INDEX_INTERNAL_SEGMENT_H_
#define KEYVI_INDEX_INTERNAL_SEGMENT_H_

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "dictionary/dictionary.h"

namespace keyvi {
namespace index {
namespace internal {

class Segment final {
 public:
  explicit Segment(const boost::filesystem::path& path, const bool load = true)
      : path_(path),
        filename_(path.filename().string()),
        deleted_keys_(),
        dictionary_(),
        in_merge_(false),
        new_delete_(false) {
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

  void MarkMerge() {
    in_merge_ = true;
    Persist();
  }

  void UnMarkMerge() {
    in_merge_ = false;
    deleted_keys_.insert(deleted_keys_during_merge_.begin(), deleted_keys_during_merge_.end());
    deleted_keys_during_merge_.clear();
  }

  bool MarkedForMerge() const { return in_merge_; }

  void DeleteKey(const std::string& key) {
    if (in_merge_) {
      deleted_keys_during_merge_.insert(key);
    } else {
      deleted_keys_.insert(key);
    }
    new_delete_ = true;
  }

  // persist deleted keys
  void Persist() {
    if (!new_delete_) {
      return;
    }

    boost::filesystem::path deleted_keys_file = path_;

    // its ensured that before merge persis is called, so we have to persist only one or the other file
    if (in_merge_) {
      deleted_keys_file += ".dkm";
      std::ofstream out_stream(deleted_keys_file.string(), std::ios::binary);
      msgpack::pack(out_stream, deleted_keys_during_merge_);
    } else {
      deleted_keys_file += ".dk";
      std::ofstream out_stream(deleted_keys_file.string(), std::ios::binary);
      msgpack::pack(out_stream, deleted_keys_);
    }
  }

 private:
  boost::filesystem::path path_;
  std::string filename_;
  std::set<std::string> deleted_keys_;
  std::set<std::string> deleted_keys_during_merge_;
  dictionary::dictionary_t dictionary_;
  bool in_merge_;
  bool new_delete_;

  void Load() { dictionary_.reset(new dictionary::Dictionary(path_.string())); }
};

typedef std::shared_ptr<Segment> segment_t;
typedef std::vector<segment_t> segment_vec_t;
typedef std::shared_ptr<std::vector<segment_t>> segments_t;

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_SEGMENT_H_
