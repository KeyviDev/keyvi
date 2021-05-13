/* * keyvi - A key value store.
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
 * comparable_state_traverser.h
 *
 *  Created on: Dec 20, 2020
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_COMPARABLE_STATE_TRAVERSER_H_
#define KEYVI_DICTIONARY_FSA_COMPARABLE_STATE_TRAVERSER_H_

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
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

template <class zipInnerTraverserType>
class ZipStateTraverser;

/**
 * Traverser wrapper that can be used to
 *
 * - track/record all states
 * - compare 2 traverser objects based on lexicographic order
 *
 * Specializations:
 * - inner weights: compare 2 traverser objects based on inner weights
 * - near traverser
 */
template <class innerTraverserType>
class ComparableStateTraverser final {
 public:
  using label_t = typename innerTraverserType::label_t;
  using transition_t = typename innerTraverserType::transition_t;

  explicit ComparableStateTraverser(const innerTraverserType &&traverser, const bool advance = true,
                                    const size_t order = 0)
      : state_traverser_(std::move(traverser)), order_(order) {
    if (advance) {
      this->operator++(0);
    }
  }

  explicit ComparableStateTraverser(const automata_t f, uint64_t start_state, const bool advance = true,
                                    const size_t order = 0)
      : state_traverser_(f, start_state, false), order_(order) {
    if (advance) {
      this->operator++(0);
    }
  }

  explicit ComparableStateTraverser(const automata_t f, bool advance = true, size_t order = 0)
      : ComparableStateTraverser(f, f->GetStartState(), advance, order) {}

  explicit ComparableStateTraverser(const automata_t f, const uint64_t start_state,
                                    traversal::TraversalPayload<transition_t> &&payload, const bool advance = true,
                                    const size_t order = 0)
      : state_traverser_(f, start_state, std::move(payload), false), order_(order) {
    if (advance) {
      this->operator++(0);
    }
  }

  ComparableStateTraverser() = delete;
  ComparableStateTraverser &operator=(ComparableStateTraverser const &) = delete;
  ComparableStateTraverser(const ComparableStateTraverser &that) = delete;

  /**
   * Comparison of the state traverser for the purpose of ordering them
   */
  bool operator<(const ComparableStateTraverser &rhs) const {
    int compare = std::memcmp(label_stack_.data(), rhs.label_stack_.data(),
                              std::min(label_stack_.size(), rhs.label_stack_.size()) * sizeof(label_t));
    if (compare != 0) {
      return compare < 0;
    }

    if (label_stack_.size() != rhs.label_stack_.size()) {
      return label_stack_.size() < rhs.label_stack_.size();
    }

    return order_ > rhs.order_;
  }

  bool operator>(const ComparableStateTraverser &rhs) const { return rhs.operator<(*this); }

  bool operator<=(const ComparableStateTraverser &rhs) const { return !operator>(rhs); }

  bool operator>=(const ComparableStateTraverser &rhs) const { return !operator<(rhs); }

  /**
   * Compare traverser with another one, _ignoring_ the order value
   */
  bool operator==(const ComparableStateTraverser &rhs) const {
    if (label_stack_.size() != rhs.label_stack_.size()) {
      return false;
    }

    return std::memcmp(label_stack_.data(), rhs.label_stack_.data(), label_stack_.size() * sizeof(label_t)) == 0;
  }

  bool operator!=(const ComparableStateTraverser &rhs) const { return !operator==(rhs); }

  operator bool() const { return state_traverser_; }

  bool AtEnd() const { return state_traverser_.AtEnd(); }

  void operator++(int) {
    state_traverser_++;
    if (state_traverser_) {
      label_stack_.resize(state_traverser_.GetDepth() - 1);
      label_stack_.push_back(state_traverser_.GetStateLabel());
    }
  }

  automata_t GetFsa() const { return state_traverser_.GetFsa(); }

  bool IsFinalState() const { return state_traverser_.IsFinalState(); }

  size_t GetDepth() const { return state_traverser_.GetDepth(); }

  uint64_t GetStateValue() const { return state_traverser_.GetStateValue(); }

  uint32_t GetInnerWeight() { return state_traverser_.GetInnerWeight(); }

  uint64_t GetStateId() const { return state_traverser_.GetStateId(); }

  void Prune() {
    state_traverser_.Prune();
    label_stack_.pop_back();
  }

  label_t GetStateLabel() const { return state_traverser_.GetStateLabel(); }

  const std::vector<label_t> &GetStateLabels() const { return label_stack_; }

  size_t GetOrder() const { return order_; }

 private:
  innerTraverserType state_traverser_;
  std::vector<label_t> label_stack_;
  size_t order_;

  template <class zipInnerTraverserType>
  friend class ZipStateTraverser;

  template <class nearInnerTraverserType>
  friend class matching::NearMatching;

  traversal::TraversalState<transition_t> &GetStates() { return state_traverser_.GetStates(); }

  traversal::TraversalPayload<transition_t> &GetTraversalPayload() { return state_traverser_.GetTraversalPayload(); }

  const traversal::TraversalPayload<transition_t> &GetTraversalPayload() const {
    return state_traverser_.GetTraversalPayload();
  }
};

inline bool CompareWeights(const traversal::TraversalState<traversal::WeightedTransition> &i,
                           const traversal::TraversalState<traversal::WeightedTransition> &j) {
  return i.GetNextInnerWeight() == j.GetNextInnerWeight();
}

template <>
inline bool ComparableStateTraverser<WeightedStateTraverser>::operator<(const ComparableStateTraverser &rhs) const {
  TRACE("operator< (weighted state specialization)");

  TRACE("depth %ld %ld", state_traverser_.GetDepth(), rhs.state_traverser_.GetDepth());

  if (state_traverser_.GetDepth() > 0 && rhs.state_traverser_.GetDepth() > 0) {
    auto compare_weights = std::mismatch(state_traverser_.GetStack().traversal_states.begin(),
                                         state_traverser_.GetStack().traversal_states.begin() +
                                             std::min(state_traverser_.GetDepth(), rhs.state_traverser_.GetDepth()) - 1,
                                         rhs.state_traverser_.GetStack().traversal_states.begin(), CompareWeights);
    if ((*compare_weights.first).GetNextInnerWeight() != (*compare_weights.second).GetNextInnerWeight()) {
      return (*compare_weights.first).GetNextInnerWeight() > (*compare_weights.second).GetNextInnerWeight();
    }
  }

  int compare = std::memcmp(label_stack_.data(), rhs.label_stack_.data(),
                            std::min(label_stack_.size(), rhs.label_stack_.size()) * sizeof(label_t));
  if (compare != 0) {
    return compare < 0;
  }

  if (label_stack_.size() != rhs.label_stack_.size()) {
    TRACE("different sizes %ld vs %ld", label_stack_.size(), rhs.label_stack_.size());
    return label_stack_.size() < rhs.label_stack_.size();
  }

  return order_ > rhs.order_;
}

template <>
inline bool ComparableStateTraverser<NearStateTraverser>::operator==(const ComparableStateTraverser &rhs) const {
  if (label_stack_.size() != rhs.label_stack_.size()) {
    return false;
  }

  if (GetTraversalPayload().exact != rhs.GetTraversalPayload().exact) {
    return false;
  }

  return std::memcmp(label_stack_.data(), rhs.label_stack_.data(), label_stack_.size() * sizeof(label_t)) == 0;
}

template <>
inline bool ComparableStateTraverser<NearStateTraverser>::operator<(const ComparableStateTraverser &rhs) const {
  TRACE("operator< (near state specialization)");

  if (GetTraversalPayload().exact != rhs.GetTraversalPayload().exact) {
    return GetTraversalPayload().exact;
  }

  if (GetTraversalPayload().exact) {
    if (GetTraversalPayload().exact_depth != rhs.GetTraversalPayload().exact_depth) {
      return GetTraversalPayload().exact_depth < rhs.GetTraversalPayload().exact_depth;
    }
  } else {
    if (GetTraversalPayload().exact_depth != rhs.GetTraversalPayload().exact_depth) {
      return GetTraversalPayload().exact_depth > rhs.GetTraversalPayload().exact_depth;
    }

    int compare = std::memcmp(label_stack_.data(), rhs.label_stack_.data(),
                              std::min(label_stack_.size(), rhs.label_stack_.size()) * sizeof(label_t));
    if (compare != 0) {
      return compare < 0;
    }

    if (label_stack_.size() != rhs.label_stack_.size()) {
      return label_stack_.size() < rhs.label_stack_.size();
    }
  }

  return order_ > rhs.order_;
}

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_COMPARABLE_STATE_TRAVERSER_H_
