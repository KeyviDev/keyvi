/* keyvi - A key value store.
 *
 * Copyright 2020 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * zip_state_traverser.h
 *
 *  Created on: Dec 16, 2020
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_ZIP_STATE_TRAVERSER_H_
#define KEYVI_DICTIONARY_FSA_ZIP_STATE_TRAVERSER_H_

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <vector>

#include <boost/heap/skew_heap.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/comparable_state_traverser.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

template <class innerTraverserType>
class ZipStateTraverser final {
 private:
  using traverser_t = std::shared_ptr<ComparableStateTraverser<innerTraverserType>>;

  struct TraverserCompare {
    bool operator()(const traverser_t &t1, const traverser_t &t2) const { return *t1 > *t2; }
  };

 public:
  using label_t = typename innerTraverserType::label_t;
  using heap_t =
      boost::heap::skew_heap<traverser_t, boost::heap::compare<TraverserCompare>, boost::heap::mutable_<true>>;
  explicit ZipStateTraverser(const std::vector<automata_t> &fsas, bool advance = true) {
    size_t order = 0;
    for (const automata_t &f : fsas) {
      traverser_queue_.emplace(std::make_shared<ComparableStateTraverser<innerTraverserType>>(f, true, order++));
    }

    if (advance) {
      this->operator++(0);
    }
  }

  explicit ZipStateTraverser(const std::initializer_list<automata_t> fsas, bool advance = true) {
    size_t order = 0;
    for (auto f : fsas) {
      traverser_queue_.emplace(std::make_shared<ComparableStateTraverser<innerTraverserType>>(f, true, order++));
    }

    if (advance) {
      this->operator++(0);
    }
  }

  ZipStateTraverser() = delete;
  ZipStateTraverser &operator=(ZipStateTraverser const &) = delete;
  ZipStateTraverser(const ZipStateTraverser &that) = delete;

  void operator++(int) {
    TRACE("iterator++");

    if (!traverser_queue_.empty()) {
      const traverser_t &t = traverser_queue_.top();
      auto it = traverser_queue_.begin();
      it++;

      final_ = t->IsFinalState();
      depth_ = t->GetDepth();
      state_value_ = t->GetStateValue();
      inner_weight_ = t->GetInnerWeight();
      state_id_ = t->GetStateId();
      state_label_ = t->GetStateLabel();

      while (it != traverser_queue_.end() && *t == *(*it)) {
        TRACE("advance 2nd: %ld", (*it)->GetOrder());

        // if not final yet check if other states are final
        if (!final_ && (*it)->IsFinalState()) {
          final_ = true;
          state_value_ = (*it)->GetStateValue();
        }

        // take the max from inner weights
        if ((*it)->GetInnerWeight() > inner_weight_) {
          inner_weight_ = (*it)->GetInnerWeight();
        }

        (*it)->operator++(0);
        if (*(*it)) {
          traverser_queue_.decrease(heap_t::s_handle_from_iterator(it));
        } else {
          traverser_queue_.erase(heap_t::s_handle_from_iterator(it));
        }
        it = traverser_queue_.begin();
        it++;
      }

      t->operator++(0);
      if (*t) {
        traverser_queue_.decrease(heap_t::s_handle_from_iterator(traverser_queue_.begin()));
      } else {
        traverser_queue_.erase(heap_t::s_handle_from_iterator(traverser_queue_.begin()));
      }
    }
  }

  operator bool() const { return !traverser_queue_.empty(); }

  bool IsFinalState() const { return final_; }

  size_t GetDepth() const { return depth_; }

  uint64_t GetStateValue() const { return state_value_; }

  uint32_t GetInnerWeight() const { return inner_weight_; }

  uint64_t GetStateId() const { return state_id_; }

  void Prune() { /*todo*/
  }

  label_t GetStateLabel() const { return state_label_; }

 private:
  heap_t traverser_queue_;
  bool final_ = false;
  size_t depth_ = 0;
  uint64_t state_value_ = 0;
  uint32_t inner_weight_ = 0;
  uint64_t state_id_ = 0;
  label_t state_label_ = 0;
};

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_ZIP_STATE_TRAVERSER_H_
