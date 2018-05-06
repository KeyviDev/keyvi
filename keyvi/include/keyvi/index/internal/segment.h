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

#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "dictionary/dictionary.h"
#include "index/internal/read_only_segment.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class Segment final : public ReadOnlySegment {
 public:
  explicit Segment(const boost::filesystem::path& path, bool no_deletes = false)
      : ReadOnlySegment(path, false, !no_deletes),
        deleted_keys_for_write_(),
        deleted_keys_during_merge_for_write_(),
        dictionary_loaded(false),
        deletes_loaded(no_deletes),
        in_merge_(false),
        new_delete_(false),
        deleted_keys_swap_filename_(path) {
    deleted_keys_swap_filename_ += ".dk-swap";
  }

  explicit Segment(const boost::filesystem::path& path, const std::vector<std::shared_ptr<Segment>>& parent_segments)
      : ReadOnlySegment(path, false, false),
        deleted_keys_for_write_(),
        deleted_keys_during_merge_for_write_(),
        dictionary_loaded(false),
        deletes_loaded(true),
        in_merge_(false),
        new_delete_(false),
        deleted_keys_swap_filename_(path) {
    deleted_keys_swap_filename_ += ".dk-swap";

    // move deletions that happened during merge into the list of deleted keys
    for (const auto& p_segment : parent_segments) {
      deleted_keys_for_write_.insert(p_segment->deleted_keys_during_merge_for_write_.begin(),
                                     p_segment->deleted_keys_during_merge_for_write_.end());
    }

    // persist the current list of deleted keys
    if (deleted_keys_for_write_.size()) {
      new_delete_ = true;
      Persist();
    }
  }

  dictionary::dictionary_t& operator*() {
    LazyLoadDictionary();
    return ReadOnlySegment::GetDictionary();
  }

  dictionary::dictionary_t& GetDictionary() {
    LazyLoadDictionary();
    return ReadOnlySegment::GetDictionary();
  }

  bool HasDeletedKeys() {
    LazyLoadDeletedKeys();
    return ReadOnlySegment::HasDeletedKeys();
  }

  const std::shared_ptr<std::unordered_set<std::string>> DeletedKeys() {
    LazyLoadDeletedKeys();
    return ReadOnlySegment::DeletedKeys();
  }

  bool IsDeleted(const std::string& key) {
    LazyLoadDeletedKeys();
    return ReadOnlySegment::IsDeleted(key);
  }

  void ElectedForMerge() {
    Persist();
    in_merge_ = true;
  }

  void MergeFailed() {
    in_merge_ = false;
    if (deleted_keys_during_merge_for_write_.size() > 0) {
      deleted_keys_for_write_.insert(deleted_keys_during_merge_for_write_.begin(),
                                     deleted_keys_during_merge_for_write_.end());

      new_delete_ = true;
      Persist();
      deleted_keys_during_merge_for_write_.clear();
      // remove dkm file
      std::remove(GetDeletedKeysDuringMergePath().string().c_str());
    }
  }

  bool MarkedForMerge() const { return in_merge_; }

  void RemoveFiles() {
    // delete files, not all files might exist, therefore ignore the output
    std::remove(GetDictionaryPath().c_str());
    std::remove(GetDeletedKeysDuringMergePath().string().c_str());
    std::remove(GetDeletedKeysPath().string().c_str());
  }

  void DeleteKey(const std::string& key) {
    if (!GetDictionary()->Contains(key)) {
      return;
    }

    // load deleted keys as well
    LazyLoadDeletedKeys();

    if (in_merge_) {
      TRACE("delete key (in merge) %s", key.c_str());
      deleted_keys_during_merge_for_write_.insert(key);
    } else {
      TRACE("delete key (no merge) %s", key.c_str());
      deleted_keys_for_write_.insert(key);
    }
    new_delete_ = true;
  }

  // persist deleted keys
  bool Persist() {
    if (!new_delete_) {
      return false;
    }
    TRACE("persist deleted keys");
    boost::filesystem::path deleted_keys_file = GetDictionaryPath();

    // its ensured that before merge persist is called, so we have to persist only one or the other file
    if (in_merge_) {
      SaveDeletedKeys(GetDeletedKeysDuringMergePath().string(), deleted_keys_during_merge_for_write_);
    } else {
      SaveDeletedKeys(GetDeletedKeysPath().string(), deleted_keys_for_write_);
    }

    return true;
  }

 private:
  std::unordered_set<std::string> deleted_keys_for_write_;
  std::unordered_set<std::string> deleted_keys_during_merge_for_write_;
  bool dictionary_loaded;
  bool deletes_loaded;
  bool in_merge_;
  bool new_delete_;
  boost::filesystem::path deleted_keys_swap_filename_;

  inline void LazyLoadDictionary() {
    if (!dictionary_loaded) {
      LoadDictionary();
      dictionary_loaded = true;
    }
  }

  inline void LazyLoadDeletedKeys() {
    if (!deletes_loaded) {
      LoadDeletedKeys();

      // get a copy of the deleted keys for writing
      if (ReadOnlySegment::HasDeletedKeys()) {
        if (in_merge_) {
          deleted_keys_during_merge_for_write_.insert(DeletedKeysDirect().begin(), DeletedKeysDirect().end());
        } else {
          deleted_keys_for_write_.insert(DeletedKeysDirect().begin(), DeletedKeysDirect().end());
        }
      }
      deletes_loaded = true;
    }
  }

  void SaveDeletedKeys(const std::string& filename, const std::unordered_set<std::string>& deleted_keys) {
    // write to swap file, than rename it
    {
      std::ofstream out_stream(deleted_keys_swap_filename_.string(), std::ios::binary);
      msgpack::pack(out_stream, deleted_keys);
    }
    std::rename(deleted_keys_swap_filename_.string().c_str(), filename.c_str());
  }
};  // namespace internal

typedef std::shared_ptr<Segment> segment_t;
typedef std::vector<segment_t> segment_vec_t;
typedef std::shared_ptr<segment_vec_t> segments_t;
typedef const std::shared_ptr<segment_vec_t> const_segments_t;

}  // namespace internal
}  // namespace index
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_SEGMENT_H_
