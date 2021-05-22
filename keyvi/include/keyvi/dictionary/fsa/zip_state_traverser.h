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
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/heap/skew_heap.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/comparable_state_traverser.h"
#include "keyvi/dictionary/fsa/traverser_types.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace matching {

template <class nearInnerTraverserType>
class NearMatching;
}
namespace fsa {

/**
 * A traverser that wraps a list of inner traversers to traverse them as if they would be 1 traverser
 *
 * The order of inner traversers define the precedence. If 2 traversers have a common traversal stack,
 * the later dictionary is taken for reading out the values/weights etc.
 *
 * Note: wrapping weighted state traverser is not supported yet
 */
template <class innerTraverserType>
class ZipStateTraverser final {
 private:
  using traverser_t = std::shared_ptr<ComparableStateTraverser<innerTraverserType>>;

  struct TraverserCompare {
    bool operator()(const traverser_t &t1, const traverser_t &t2) const { return *t1 > *t2; }
  };

 public:
  using label_t = typename innerTraverserType::label_t;
  using transition_t = typename innerTraverserType::transition_t;
  using heap_t =
      boost::heap::skew_heap<traverser_t, boost::heap::compare<TraverserCompare>, boost::heap::mutable_<true>>;
  explicit ZipStateTraverser(const std::vector<automata_t> &fsas, const bool advance = true) {
    size_t order = 0;
    for (const automata_t &f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<innerTraverserType>>(f, advance, order++);
      // the traverser could be exhausted after it has been advanced
      if (*traverser) {
        traverser_queue_.push(traverser);
      }
    }
    FillInValues();
  }

  explicit ZipStateTraverser(const std::initializer_list<automata_t> fsas, const bool advance = true) {
    size_t order = 0;
    for (auto f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<innerTraverserType>>(f, advance, order++);
      // the traverser could be exhausted after it has been advanced
      if (*traverser) {
        traverser_queue_.push(traverser);
      }
    }
    FillInValues();
  }

  explicit ZipStateTraverser(const std::vector<std::pair<automata_t, uint64_t>> &fsa_start_state_pairs,
                             const bool advance = true) {
    size_t order = 0;
    for (auto f : fsa_start_state_pairs) {
      if (f.second > 0) {
        traverser_t traverser =
            std::make_shared<ComparableStateTraverser<innerTraverserType>>(f.first, f.second, advance, order++);
        // the traverser could be exhausted after it has been advanced
        if (*traverser) {
          traverser_queue_.push(traverser);
        }
      }
    }
    FillInValues();
  }

  explicit ZipStateTraverser(std::vector<std::tuple<automata_t, uint64_t, traversal::TraversalPayload<transition_t>>>
                                 &&fsa_start_state_payloads,
                             const bool advance = true) {
    size_t order = 0;

    for (auto &f : fsa_start_state_payloads) {
      if (std::get<1>(f) > 0) {
        traverser_t traverser = std::make_shared<ComparableStateTraverser<innerTraverserType>>(
            std::get<0>(f), std::get<1>(f), std::move(std::get<2>(f)), advance, order++);

        // the traverser could be exhausted after it has been advanced
        if (*traverser) {
          traverser_queue_.push(traverser);
        }
      }
    }
    FillInValues();
  }

  ZipStateTraverser() = delete;
  ZipStateTraverser &operator=(ZipStateTraverser const &) = delete;
  ZipStateTraverser(const ZipStateTraverser &that) = delete;

  ZipStateTraverser(ZipStateTraverser &&other)
      : traverser_queue_(std::move(other.traverser_queue_)),
        final_(other.final_),
        depth_(other.depth_),
        state_value_(other.state_value_),
        inner_weight_(other.inner_weight_),
        state_id_(other.state_id_),
        state_label_(other.state_label_),
        order_(other.order_),
        fsa_(std::move(other.fsa_)),
        equal_states_(other.equal_states_) {
    other.final_ = false;
    other.depth_ = 0;
    other.state_value_ = 0;
    other.inner_weight_ = 0;
    other.state_id_ = 0;
    other.state_label_ = 0;
    other.order_ = 0;
    other.equal_states_ = 1;
  }

  void operator++(int) {
    TRACE("iterator++, forwarding %ld inner traversers", equal_states_);

    PreIncrement();

    while (equal_states_ > 0) {
      // get the top element
      auto it = traverser_queue_.begin();
      TRACE("++ iterator: %ld", (*it)->GetOrder());
      // advance the inner traverser and update or remove it from the queue
      (*it)->operator++(0);
      if (*(*it)) {
        traverser_queue_.decrease(heap_t::s_handle_from_iterator(it));
      } else {
        traverser_queue_.erase(heap_t::s_handle_from_iterator(it));
      }
      --equal_states_;
    }
    FillInValues();
  }

  operator bool() const { return !traverser_queue_.empty(); }

  bool AtEnd() const { return traverser_queue_.empty(); }

  automata_t GetFsa() const { return fsa_; }

  bool IsFinalState() const { return final_; }

  size_t GetDepth() const { return depth_; }

  uint64_t GetStateValue() const { return state_value_; }

  uint32_t GetInnerWeight() const { return inner_weight_; }

  uint64_t GetStateId() const { return state_id_; }

  size_t GetOrder() const { return order_; }

  void Prune() {
    TRACE("pruning %ld inner traversers", equal_states_);
    size_t to_prune = equal_states_;

    auto it = traverser_queue_.ordered_begin();
    while (to_prune > 0) {
      TRACE("prune traverser %lu", (*it)->GetOrder());
      (*it++)->Prune();
      --to_prune;
    }
    pruned = true;
  }

  label_t GetStateLabel() const { return state_label_; }

  const std::vector<label_t> &GetStateLabels() const { return traverser_queue_.top()->GetStateLabels(); }

 private:
  heap_t traverser_queue_;
  bool final_ = false;
  size_t depth_ = 0;
  uint64_t state_value_ = 0;
  uint32_t inner_weight_ = 0;
  uint64_t state_id_ = 0;
  label_t state_label_ = 0;
  size_t order_;
  automata_t fsa_;
  size_t equal_states_ = 1;
  bool pruned = false;

  inline void PreIncrement() {}

  void FillInValues() {
    TRACE("fill in values");
    pruned = false;

    if (!traverser_queue_.empty()) {
      const traverser_t &t = traverser_queue_.top();
      TRACE("take values from traverser %lu", t->GetOrder());

      final_ = t->IsFinalState();
      depth_ = t->GetDepth();
      state_value_ = t->GetStateValue();
      inner_weight_ = t->GetInnerWeight();
      state_id_ = t->GetStateId();
      state_label_ = t->GetStateLabel();
      fsa_ = t->GetFsa();
      order_ = t->GetOrder();
      // memorize how many inner traverser are at a equal inner state
      equal_states_ = 1;

      // traverse the queue in _sorted_ order
      auto it = traverser_queue_.ordered_begin();
      it++;

      TRACE("label: %c", state_label_);

      while (traverser_queue_.size() > equal_states_ && *t == *(*it)) {
        TRACE("dedup");
        equal_states_++;
        // if not final yet check if other states are final
        if (!final_ && (*it)->IsFinalState()) {
          TRACE("found final state in traverser %lu", (*it)->GetOrder());
          final_ = true;
          state_value_ = (*it)->GetStateValue();
          fsa_ = (*it)->GetFsa();
          order_ = (*it)->GetOrder();
        }

        it++;
      }

    } else {
      // reset values
      final_ = false;
      depth_ = 0;
      state_value_ = 0;
      inner_weight_ = 0;
      state_id_ = 0;
      state_label_ = 0;
      fsa_.reset();
    }
  }

  template <class nearInnerTraverserType>
  friend class matching::NearMatching;

  const traversal::TraversalPayload<transition_t> &GetTraversalPayload() const {
    return traverser_queue_.top()->GetTraversalPayload();
  }
};

template <>
inline void ZipStateTraverser<WeightedStateTraverser>::PreIncrement() {
  TRACE("preincrement weighted state specialization");

  // don't patch weight if prune has been called before
  if (pruned) {
    TRACE("traverser has been pruned, skipping preincrement");
    return;
  }

  // patch weights if necessary
  // there are 2 or more traverser with different weights, we are creating a global weights map
  // and patch every traverser with the same weight, afterwards we re-sort the transitions, so that
  // we get consistent order
  if (equal_states_ > 1) {
    std::map<label_t, uint32_t> global_weights;

    // 1st pass
    auto it = traverser_queue_.ordered_begin();
    size_t steps = equal_states_;

    while (steps > 0) {
      for (const transition_t &transition : (*it)->GetStates().traversal_state_payload.transitions) {
        if (global_weights.count(transition.label) == 0 || global_weights.at(transition.label) < transition.weight) {
          global_weights[transition.label] = transition.weight;
        }
      }
      it++;
      --steps;
    }

    // 2nd pass
    it = traverser_queue_.ordered_begin();
    steps = equal_states_;
    while (steps > 0) {
      for (transition_t &transition : (*it)->GetStates().traversal_state_payload.transitions) {
        transition.weight = global_weights.at(transition.label);
      }
      // re-sort transitions
      (*it)->GetStates().PostProcess(&(*it)->GetTraversalPayload());
      it++;
      --steps;
    }
  }
}

template <>
inline ZipStateTraverser<WeightedStateTraverser>::ZipStateTraverser(const std::initializer_list<automata_t> fsas,
                                                                    const bool advance) {
  TRACE("construct  (weighted state specialization)");
  size_t order = 0;

  if (fsas.size() < 2) {
    for (auto f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f, advance, order++);
      // the traverser could be exhausted after it has been advanced
      if (*traverser) {
        traverser_queue_.push(traverser);
      }
    }
  } else {
    std::map<label_t, uint32_t> global_weights;
    std::vector<traverser_t> traversers;

    for (auto f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f, false, order++);
      traversers.push_back(traverser);
    }

    // 1st pass collect all weights per label
    for (const auto &t : traversers) {
      for (const transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        if (global_weights.count(transition.label) == 0 || global_weights.at(transition.label) < transition.weight) {
          global_weights[transition.label] = transition.weight;
        }
      }
    }
    // 2nd pass apply global weights
    for (const auto &t : traversers) {
      for (transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        transition.weight = global_weights.at(transition.label);
      }
      // re-sort transitions
      t->GetStates().PostProcess(&t->GetTraversalPayload());

      // now advance?
      if (advance) {
        t->operator++(0);
      }
      // the traverser could be exhausted after it has been advanced
      if (*t) {
        traverser_queue_.push(t);
      }
    }
  }

  FillInValues();
}

template <>
inline ZipStateTraverser<WeightedStateTraverser>::ZipStateTraverser(const std::vector<automata_t> &fsas,
                                                                    const bool advance) {
  TRACE("construct  (weighted state specialization)");
  size_t order = 0;

  if (fsas.size() < 2) {
    for (auto f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f, advance, order++);
      // the traverser could be exhausted after it has been advanced
      if (*traverser) {
        traverser_queue_.push(traverser);
      }
    }
  } else {
    std::map<label_t, uint32_t> global_weights;
    std::vector<traverser_t> traversers;

    for (auto f : fsas) {
      traverser_t traverser = std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f, false, order++);
      traversers.push_back(traverser);
    }

    // 1st pass collect all weights per label
    for (const auto &t : traversers) {
      for (const transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        if (global_weights.count(transition.label) == 0 || global_weights.at(transition.label) < transition.weight) {
          global_weights[transition.label] = transition.weight;
        }
      }
    }
    // 2nd pass apply global weights
    for (const auto &t : traversers) {
      for (transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        transition.weight = global_weights.at(transition.label);
      }
      // re-sort transitions
      t->GetStates().PostProcess(&t->GetTraversalPayload());

      // now advance?
      if (advance) {
        t->operator++(0);
      }
      // the traverser could be exhausted after it has been advanced
      if (*t) {
        traverser_queue_.push(t);
      }
    }
  }

  FillInValues();
}

template <>
inline ZipStateTraverser<WeightedStateTraverser>::ZipStateTraverser(
    const std::vector<std::pair<automata_t, uint64_t>> &fsa_start_state_pairs, const bool advance) {
  size_t order = 0;

  if (fsa_start_state_pairs.size() < 2) {
    for (auto f : fsa_start_state_pairs) {
      if (f.second > 0) {
        traverser_t traverser =
            std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f.first, f.second, advance, order++);
        // the traverser could be exhausted after it has been advanced
        if (*traverser) {
          traverser_queue_.push(traverser);
        }
      }
    }
  } else {
    // there is more than 1 inner traverser
    std::map<label_t, uint32_t> global_weights;
    std::vector<traverser_t> traversers;
    for (auto f : fsa_start_state_pairs) {
      if (f.second > 0) {
        traverser_t traverser =
            std::make_shared<ComparableStateTraverser<WeightedStateTraverser>>(f.first, f.second, false, order++);
        traversers.push_back(traverser);
      }
    }
    // 1st pass collect all weights per label
    for (const auto &t : traversers) {
      for (const transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        if (global_weights.count(transition.label) == 0 || global_weights.at(transition.label) < transition.weight) {
          global_weights[transition.label] = transition.weight;
        }
      }
    }
    // 2nd pass apply global weights
    for (const auto &t : traversers) {
      for (transition_t &transition : t->GetStates().traversal_state_payload.transitions) {
        transition.weight = global_weights.at(transition.label);
      }
      TRACE("resort %ld", t->GetOrder());

      // re-sort transitions
      t->GetStates().PostProcess(&t->GetTraversalPayload());

      // now advance?
      if (advance) {
        t->operator++(0);
      }

      // the traverser could be exhausted after it has been advanced
      if (*t) {
        traverser_queue_.push(t);
      }
    }
  }
  FillInValues();
}

}  // namespace fsa
}  // namespace dictionary
}  // namespace keyvi

#endif  // KEYVI_DICTIONARY_FSA_ZIP_STATE_TRAVERSER_H_
