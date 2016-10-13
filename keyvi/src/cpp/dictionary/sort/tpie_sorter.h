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

#ifndef TPIE_SORTER_H_
#define TPIE_SORTER_H_

#include <boost/filesystem.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "tpie/serialization_sorter.h"
#include "dictionary/util/tpie_initializer.h"
#include "dictionary/sort/sorter_common.h"
#include "dictionary/fsa/internal/constants.h"
//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace sort {

/**
 * Tpie serialization and deserialization for sorting.
 */
template<typename Dst, typename KeyValueT>
void serialize(Dst & d, const KeyValueT & pt) {
  using tpie::serialize;
  serialize(d, pt.key);
  serialize(d, pt.value);
}
template<typename Src, typename KeyValueT>
void unserialize(Src & s, KeyValueT & pt) {
  using tpie::unserialize;
  unserialize(s, pt.key);
  unserialize(s, pt.value);
}

template<typename KeyValueT>
class TpieSorter final {

 public:
  typedef tpie::serialization_sorter<KeyValueT> tpie_sorter_t;

  class TpieSortIterator final : public boost::iterator_facade<TpieSortIterator  // CRTP, just use the Iterator name
      , KeyValueT const  // Value type of what is iterated over (contained element type)
      , boost::single_pass_traversal_tag  // type of traversal allowed
      >// Reference and Difference can be omitted
  {

   public:
    TpieSortIterator(): sorter_(), at_end_(true) {
    }

    TpieSortIterator(tpie_sorter_t* sorter)
        : sorter_(sorter), at_end_(false) {
      increment();
    }

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

    KeyValueT const & dereference() const {
      return current_;
    }

  };
  /**
   * Instantiate a TPIE sorter (external memory sort).
   *
   * @param memory_limit memory limit for internal memory usage
   */
  TpieSorter(size_t memory_limit = 1073741824, const sorter_param_t& params = sorter_param_t())
      : initializer_(util::TpieIntializer::getInstance()),
        sorter_(),
        params_(params) {

    sorter_.set_available_memory(memory_limit);
    sorter_.begin();

    if (params_.count(TEMPORARY_PATH_KEY) == 0) {
      params_[TEMPORARY_PATH_KEY] = boost::filesystem::temp_directory_path()
          .string();
    }

    initializer_.SetTempDirectory(params_[TEMPORARY_PATH_KEY]);
  }

  void push_back(const KeyValueT& kv) {
    sorter_.push(kv);
  }

  void sort() {
    sorter_.end();
    sorter_.merge_runs();
  }

  size_t size() {
    return sorter_.item_count();
  }

  TpieSortIterator begin() {
    return TpieSortIterator(&sorter_);
  }

  TpieSortIterator end() {
    return TpieSortIterator();
  }

  void clear() {
    sorter_.evacuate();
  }

 private:
  util::TpieIntializer& initializer_;
  tpie_sorter_t sorter_;
  sorter_param_t params_;
};

} /* namespace sort */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* TPIE_SORTER_H_ */
