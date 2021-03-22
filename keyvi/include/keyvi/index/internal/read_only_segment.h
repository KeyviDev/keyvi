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
#include <mutex>  //NOLINT
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>

#include <msgpack.hpp>

#include "keyvi/dictionary/dictionary.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace index {
namespace internal {

class ReadOnlySegment {
 public:
  using deleted_t = std::unordered_set<std::string>;
  using deleted_ptr_t = std::shared_ptr<deleted_t>;

  explicit ReadOnlySegment(const boost::filesystem::path& path)
      : dictionary_path_(path),
        dictionary_properties_(std::make_shared<dictionary::DictionaryProperties>(
            dictionary::DictionaryProperties::FromFile(path.string()))),
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

    LoadDictionary();
    LoadDeletedKeys();
  }

  dictionary::dictionary_t& operator*() { return dictionary_; }

  dictionary::dictionary_t& GetDictionary() { return dictionary_; }

  dictionary::dictionary_properties_t& GetDictionaryProperties() { return dictionary_properties_; }

  bool HasDeletedKeys() { return has_deleted_keys_; }

  size_t DeletedKeysSize() const {
    if (has_deleted_keys_) {
      return deleted_keys_->size();
    }
    return 0;
  }

  const deleted_ptr_t DeletedKeys() {
    if (!has_deleted_keys_) {
      return deleted_ptr_t();
    }

    deleted_ptr_t deleted_keys = deleted_keys_weak_.lock();
    if (!deleted_keys) {
      std::unique_lock<std::mutex> lock(mutex_);
      deleted_keys_weak_ = deleted_keys_;
      deleted_keys = deleted_keys_;
    }
    return deleted_keys;
  }

  bool IsDeleted(const std::string& key) {
    if (has_deleted_keys_) {
      return (DeletedKeys()->count(key) > 0);
    }

    return false;
  }

  void ReloadDeletedKeys() { LoadDeletedKeys(); }

  const boost::filesystem::path& GetDictionaryPath() const { return dictionary_path_; }

  const boost::filesystem::path& GetDeletedKeysPath() const { return deleted_keys_path_; }

  const boost::filesystem::path& GetDeletedKeysDuringMergePath() const { return deleted_keys_during_merge_path_; }

  const std::string& GetDictionaryFilename() const { return dictionary_filename_; }

 protected:
  explicit ReadOnlySegment(const boost::filesystem::path& path, bool load_dictionary, bool load_deleted_keys)
      : dictionary_path_(path),
        dictionary_properties_(std::make_shared<dictionary::DictionaryProperties>(
            dictionary::DictionaryProperties::FromFile(path.string()))),
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

    if (load_dictionary) {
      LoadDictionary();
    }

    if (load_deleted_keys) {
      LoadDeletedKeys();
    }
  }

  explicit ReadOnlySegment(const dictionary::dictionary_properties_t& dictionary_properties, bool load_dictionary,
                           bool load_deleted_keys)
      : dictionary_path_(dictionary_properties->GetFileName()),
        dictionary_properties_(dictionary_properties),
        deleted_keys_path_(dictionary_path_),
        deleted_keys_during_merge_path_(dictionary_path_),
        dictionary_filename_(dictionary_path_.filename().string()),
        dictionary_(),
        has_deleted_keys_(false),
        deleted_keys_(),
        last_modification_time_deleted_keys_(0),
        last_modification_time_deleted_keys_during_merge_(0) {
    deleted_keys_path_ += ".dk";
    deleted_keys_during_merge_path_ += ".dkm";

    if (load_dictionary) {
      LoadDictionary();
    }

    if (load_deleted_keys) {
      LoadDeletedKeys();
    }
  }

  void LoadDictionary() {
    // load dictionary
    dictionary_.reset(new dictionary::Dictionary(dictionary_path_.string()));
  }

  void LoadDeletedKeys() {
    TRACE("load deleted keys");

    boost::system::error_code ec;
    std::time_t last_write_dk = boost::filesystem::last_write_time(deleted_keys_path_, ec);
    // effectively ignore if file does not exist
    if (ec) {
      last_write_dk = last_modification_time_deleted_keys_;
    }

    std::time_t last_write_dkm = boost::filesystem::last_write_time(deleted_keys_during_merge_path_, ec);
    // effectively ignore if file does not exist
    if (ec) {
      last_write_dkm = last_modification_time_deleted_keys_during_merge_;
    }

    // if any list has changed, reload it
    if (last_write_dk > last_modification_time_deleted_keys_ ||
        last_write_dkm > last_modification_time_deleted_keys_during_merge_) {
      TRACE("found deleted keys");

      deleted_ptr_t deleted_keys = std::make_shared<deleted_t>();
      deleted_t deleted_keys_dk = LoadAndUnserializeDeletedKeys(deleted_keys_path_.string());
      TRACE("Loaded deleted keys: %d", deleted_keys_dk.size());

      deleted_keys->swap(deleted_keys_dk);
      // deleted_keys->insert(deleted_keys_dk.begin(), deleted_keys_dk.end());
      deleted_t deleted_keys_dkm = LoadAndUnserializeDeletedKeys(deleted_keys_during_merge_path_.string());
      TRACE("Loaded deleted keys m: %d", deleted_keys_dkm.size());

      deleted_keys->insert(deleted_keys_dkm.begin(), deleted_keys_dkm.end());

      // safe swap
      {
        std::unique_lock<std::mutex> lock(mutex_);
        deleted_keys_.swap(deleted_keys);
      }
      TRACE("Number of deleted keys: %d", deleted_keys_->size());

      has_deleted_keys_ = true;
    }
  }

  const deleted_t& DeletedKeysDirect() const { return *deleted_keys_; }

 private:
  //! path of the underlying dictionary
  boost::filesystem::path dictionary_path_;

  //! the properties of the dictionary
  dictionary::dictionary_properties_t dictionary_properties_;

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
  deleted_ptr_t deleted_keys_;

  //! a weak ptr to deleted keys for access in the main thread
  std::weak_ptr<deleted_t> deleted_keys_weak_;

  //! a mutex to secure access to the deleted keys shared pointer
  std::mutex mutex_;

  //! last modification time for the deleted keys file
  std::time_t last_modification_time_deleted_keys_;

  //! last modification time for the deleted keys file during a merge operation
  std::time_t last_modification_time_deleted_keys_during_merge_;

  inline static deleted_t LoadAndUnserializeDeletedKeys(const std::string& filename) {
    TRACE("loading deleted keys file %s", filename.c_str());

    deleted_t deleted_keys;
    std::ifstream deleted_keys_stream(filename, std::ios::binary);
    if (deleted_keys_stream.good()) {
      std::stringstream buffer;
      buffer << deleted_keys_stream.rdbuf();

      msgpack::unpacked unpacked_object;
      msgpack::unpack(unpacked_object, buffer.str().data(), buffer.str().size());
      unpacked_object.get().convert(deleted_keys);
    }
    return deleted_keys;
  }
};

typedef std::shared_ptr<ReadOnlySegment> read_only_segment_t;
typedef std::vector<read_only_segment_t> read_only_segment_vec_t;
typedef std::shared_ptr<read_only_segment_vec_t> read_only_segments_t;
typedef const std::shared_ptr<read_only_segment_vec_t> const_read_only_segments_t;

} /* namespace internal */
} /* namespace index */
} /* namespace keyvi */

#endif  // KEYVI_INDEX_INTERNAL_READ_ONLY_SEGMENT_H_
