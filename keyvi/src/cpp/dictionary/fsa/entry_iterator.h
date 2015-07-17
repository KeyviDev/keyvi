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
 * entry_iterator.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

#ifndef ENTRY_ITERATOR_H_
#define ENTRY_ITERATOR_H_

#include "dictionary/fsa/automata.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

// todo: right now hardcoded for int value storage
class EntryIterator {

 public:
  EntryIterator()
      : current_state_(0),
        current_value_(0),
        fsa_(0) {
  }

  EntryIterator(automata_t f)
      : fsa_(f) {
    current_state_ = f->GetStartState();

    TRACE("Get Start state %d", current_state_);

    traversal_stack_.push_back(0);
    TraverseToNextFinalState();
  }

  EntryIterator(automata_t f, int start_state)
      : fsa_(f) {
    current_state_ = start_state;

    TRACE("Get Start state %d", current_state_);

    traversal_stack_.push_back(0);
    TraverseToNextFinalState();
  }

  ~EntryIterator() {
    if (buffer_for_reuse_) {
      delete[] buffer_for_reuse_;
    }
  }

  std::string GetKey() {
    return std::string(GetKeyNoCopy());
  }

  const char* GetKeyNoCopy() {
    if (buffer_for_reuse_size_ < traversal_stack_.size()) {
      if (buffer_for_reuse_) {
        delete[] buffer_for_reuse_;
      }
      // allocate a little more the necessary
      buffer_for_reuse_ = new char[traversal_stack_.size() + 10];
      buffer_for_reuse_size_ = traversal_stack_.size() + 10;
    }

    std::copy(traversal_stack_.begin(), traversal_stack_.end(),
              buffer_for_reuse_);

    // overwrite the last character (which is a 1) with 0
    buffer_for_reuse_[traversal_stack_.size() - 1] = '\0';
    return buffer_for_reuse_;
  }

  uint64_t GetValueId() const {
    return current_value_;
  }

  internal::IValueStoreReader::attributes_t GetValueAsAttributeVector() const {
    return fsa_->GetValueAsAttributeVector(current_value_);
  }

  std::string GetValueAsString() const {
    return fsa_->GetValueAsString(current_value_);
  }

  EntryIterator &operator=(const EntryIterator &other) {
    fsa_ = other.fsa_;
    current_state_ = other.current_state_;
    current_key_ = other.current_key_;
    current_value_ = other.current_value_;
    traversal_stack_ = other.traversal_stack_;
    state_traversal_stack_ = other.state_traversal_stack_;
    current_depth_ = other.current_depth_;
    return *this;
  }

  EntryIterator &operator++() {
    TraverseToNextFinalState();
    return *this;
  }

  bool operator==(const EntryIterator &other) const {
    return (current_state_ == other.current_state_);
  }

  bool operator!=(const EntryIterator &other) const {
    return !(operator==(other));
  }

 private:
  void TraverseToNextFinalState() {

    if (current_state_ == 0) {
      return;
    }

    uint32_t child_node = 0;

    while (1) {

      do {
        ++traversal_stack_[current_depth_];
        child_node = fsa_->TryWalkTransition(current_state_,
                                             traversal_stack_[current_depth_]);

        if (traversal_stack_[current_depth_] == 255) {
          break;
        }
      } while (!child_node);

      if (child_node) {
        /* Found a valid childnode
         * go one level down
         */
        ++current_depth_;
        state_traversal_stack_.push_back(current_state_);
        current_state_ = child_node;
        traversal_stack_.push_back(1);

        if (fsa_->IsFinalState(child_node)) {
          // we found a final state
          // todo: implement value for other stores
          current_value_ = fsa_->GetStateValue(child_node);
          return;
        }
      } else {
        // did not found any more transitions at this deep

        if (current_depth_) {
          /* we did not find any path at the current level (deep)
           * go one level up
           */
          traversal_stack_.pop_back();
          current_state_ = state_traversal_stack_.back();
          state_traversal_stack_.pop_back();
          --current_depth_;
        } else {
          // we are at the very end
          current_state_ = 0;
          return;
        }
      }
    }
  }

  automata_t fsa_;
  uint32_t current_state_;
  std::string current_key_;
  uint64_t current_value_;
  std::vector<unsigned char> traversal_stack_;
  std::vector<uint32_t> state_traversal_stack_;

  char * buffer_for_reuse_ = 0;
  size_t buffer_for_reuse_size_ = 0;
  unsigned long current_depth_ = 0;
};

typedef std::shared_ptr<EntryIterator> entry_iterator_t;

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* ENTRY_ITERATOR_H_ */
