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
 * tpie_sorter.h
 *
 *  Created on: Oct 11, 2016
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_SORT_TPIE_SORTER_H_
#define KEYVI_DICTIONARY_SORT_TPIE_SORTER_H_

#include <string>

#include <boost/filesystem.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/sort/sorter_common.h"
#include "keyvi/dictionary/sort/tpie_initializer.h"
#include "keyvi/util/configuration.h"

#include "tpie/serialization_sorter.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace sort {

/**
 * Tpie serialization and deserialization for sorting.
 */

// TPIE is not able to handle nested fields, so we have to flatten them
template <typename Dst>
void serialize(Dst& d, const key_value_pair<std::string, fsa::ValueHandle>& kv) {  // NOLINT
  using tpie::serialize;
  serialize(d, kv.key);
  serialize(d, kv.value.value_idx_);
  serialize(d, kv.value.count_);
  serialize(d, kv.value.weight_);
  serialize(d, kv.value.no_minimization_);
  serialize(d, kv.value.deleted_);
}

template <typename Src>
void unserialize(Src& s, key_value_pair<std::string, fsa::ValueHandle>& kv) {  // NOLINT
  using tpie::unserialize;
  unserialize(s, kv.key);
  unserialize(s, kv.value.value_idx_);
  unserialize(s, kv.value.count_);
  unserialize(s, kv.value.weight_);
  unserialize(s, kv.value.no_minimization_);
  unserialize(s, kv.value.deleted_);
}

template <typename Dst, typename KeyValueT>
void serialize(Dst& d, const KeyValueT& kv) {  // NOLINT
  using tpie::serialize;
  serialize(d, kv.key);
  serialize(d, kv.value);
}
template <typename Src, typename KeyValueT>
void unserialize(Src& s, KeyValueT& kv) {  // NOLINT
  using tpie::unserialize;
  unserialize(s, kv.key);
  unserialize(s, kv.value);
}

template <typename KeyValueT>
class TpieSorter final {
 public:
  typedef tpie::serialization_sorter<KeyValueT> tpie_sorter_t;

  class TpieSortIterator final
      : public boost::iterator_facade<TpieSortIterator, KeyValueT const, boost::single_pass_traversal_tag> {
   public:
    TpieSortIterator() : sorter_(), at_end_(true) {}

    explicit TpieSortIterator(tpie_sorter_t* sorter) : sorter_(sorter), at_end_(false) { increment(); }

   private:
    friend class boost::iterator_core_access;
    tpie_sorter_t* sorter_;
    KeyValueT current_;
    bool at_end_;

    void increment() {
      if (sorter_->can_pull()) {
        current_ = sorter_->pull();
      } else {
        at_end_ = true;
      }
    }

    bool equal(TpieSortIterator const& other) const {
      if (at_end_) {
        return other.at_end_;
      } else if (other.at_end_) {
        return false;
      }

      return current_ == other.current_;
    }

    KeyValueT const& dereference() const { return current_; }
  };

  /**
   * Instantiate a TPIE sorter (external memory sort).
   *
   * @param memory_limit memory limit for internal memory usage
   */
  explicit TpieSorter(const sorter_param_t& params = sorter_param_t())
      : initializer_(TpieIntializer::getInstance()), sorter_(), params_(params) {
    size_t memory_limit = keyvi::util::mapGetMemory(params_, MEMORY_LIMIT_KEY, DEFAULT_MEMORY_LIMIT_TPIE_SORT);

    sorter_.set_available_memory(memory_limit);
    sorter_.begin();

    params_[TEMPORARY_PATH_KEY] =
        keyvi::util::mapGet(params_, TEMPORARY_PATH_KEY, boost::filesystem::temp_directory_path().string());

    initializer_.SetTempDirectory(params_[TEMPORARY_PATH_KEY]);
  }

  void push_back(const KeyValueT& kv) { sorter_.push(kv); }

  void sort() {
    sorter_.end();
    sorter_.merge_runs();
  }

  size_t size() { return sorter_.item_count(); }

  TpieSortIterator begin() { return TpieSortIterator(&sorter_); }

  TpieSortIterator end() { return TpieSortIterator(); }

  void clear() { sorter_.evacuate(); }

 private:
  TpieIntializer& initializer_;
  tpie_sorter_t sorter_;
  sorter_param_t params_;
};

} /* namespace sort */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_SORT_TPIE_SORTER_H_
