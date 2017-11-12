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

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "dictionary/dictionary.h"

namespace keyvi {
namespace index {
namespace internal {

class Segment final {
 public:
  explicit Segment(const boost::filesystem::path& path, const bool load = true)
      : path_(path), filename_(path.filename().string()), deleted_keys_(), dictionary_(), in_merge_(false) {
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

  const boost::filesystem::path& GetPath() const { return path_; }

  const std::string& GetFilename() const { return filename_; }

  void MarkMerge() { in_merge_ = true; }

  void UnMarkMerge() { in_merge_ = false; }

  bool MarkedForMerge() const { return in_merge_; }

  void DeleteKey(const std::string& key) {
    deleted_keys_.insert(key);
    dirty_ = true;
  }

  // persist deleted keys
  void Persist() {
    boost::filesystem::path deleted_keys_file = path_;
    deleted_keys_file += ".dk";

    std::ofstream out_stream(deleted_keys_file.string(), std::ios::binary);
    msgpack::pack(out_stream, deleted_keys_);
  }

 private:
  boost::filesystem::path path_;
  std::string filename_;
  std::set<std::string> deleted_keys_;
  dictionary::dictionary_t dictionary_;
  bool in_merge_;
  bool dirty_;

  void Load() { dictionary_.reset(new dictionary::Dictionary(path_.string())); }
};

typedef std::shared_ptr<Segment> segment_t;

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_SEGMENT_H_
