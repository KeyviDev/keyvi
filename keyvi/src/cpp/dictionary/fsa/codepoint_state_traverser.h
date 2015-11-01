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
 * codepoint_state_traverser.h
 *
 *  Created on: Jun 24, 2014
 *      Author: hendrik
 */

#ifndef CODEPOINT_STATE_TRAVERSER_H_
#define CODEPOINT_STATE_TRAVERSER_H_

#include "dictionary/fsa/automata.h"
#include "dictionary/util/utf8_utils.h"

// #define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

template<class innerTraverserType>
class CodePointStateTraverser
final {
   public:
    CodePointStateTraverser(automata_t f)
        : wrapped_state_traverser_(f, f->GetStartState(), false) {
      this->operator ++(0);
    }

    CodePointStateTraverser(automata_t f, uint64_t state)
        : wrapped_state_traverser_(f, state, false) {
      this->operator ++(0);
    }

    CodePointStateTraverser() = delete;
    CodePointStateTraverser& operator=(CodePointStateTraverser const&) = delete;
    CodePointStateTraverser(const CodePointStateTraverser& that) = delete;

    CodePointStateTraverser(CodePointStateTraverser&& other):
      wrapped_state_traverser_(std::move(other.wrapped_state_traverser_)),
      transitions_stack_(std::move(other.transitions_stack_)),
      utf8_length_stack_(std::move(other.utf8_length_stack_)),
      codepoint_(other.codepoint_),
      current_depth_(other.current_depth_)
    {
      other.current_depth_ = 0;
      other.codepoint_ = 0;
    }

    void operator++(int) {
      int remaining_bytes = 0;
      do {
        wrapped_state_traverser_++;
        int label = wrapped_state_traverser_.GetStateLabel();

        TRACE("CP traverser: wrapped traverser %x %d", label, wrapped_state_traverser_.GetDepth());

        if (label > 0) {
          PruneHistory(wrapped_state_traverser_.GetDepth() - 1);

          if (transitions_stack_.empty() || utf8_length_stack_.back() == 0) {
            // the next byte must be a utf8 leadbyte
            remaining_bytes = util::Utf8Utils::GetCharLength(label) - 1;

            ++current_depth_;
          } else {
            // this byte must be a utf8 follow byte
            remaining_bytes = utf8_length_stack_.back() - 1;
          }

          transitions_stack_.push_back(label);
          utf8_length_stack_.push_back(remaining_bytes);
        } else {

          transitions_stack_.clear();
          utf8_length_stack_.clear();
          remaining_bytes = 0;
          current_depth_ = 0;
          codepoint_ = 0;
          return;
        }

      } while (remaining_bytes > 0);

      TRACE("CP traverser: extracting codepoint");

      ExtractCodePointFromStack();
    }

    automata_t GetFsa() const {
      return wrapped_state_traverser_.GetFsa();
    }

    bool IsFinalState() {
      return  wrapped_state_traverser_.IsFinalState();
    }

    size_t GetDepth() {
      return current_depth_;
    }

    uint64_t GetStateValue() {
      return wrapped_state_traverser_.GetStateValue();
    }

    uint64_t GetStateId() {
      return wrapped_state_traverser_.GetStateId();
    }

    internal::IValueStoreReader::attributes_t GetValueAsAttributeVector() {
      return wrapped_state_traverser_.GetValueAsAttributeVector();
    }

    void Prune() {
      wrapped_state_traverser_.Prune();
      PruneHistory(wrapped_state_traverser_.GetDepth());
    }

    int GetStateLabel() {
      return codepoint_;
    }

   private:
    innerTraverserType wrapped_state_traverser_;
    std::vector<int> transitions_stack_;
    std::vector<int> utf8_length_stack_;
    int codepoint_ = 0;
    size_t current_depth_ = 0;

    void PruneHistory(size_t new_history_top) {
      while (transitions_stack_.size() > new_history_top) {
        int entry = transitions_stack_.back();

        // remove this item from the history
        transitions_stack_.pop_back();
        utf8_length_stack_.pop_back();

        if (util::Utf8Utils::IsLeadByte(entry)) {
          // We popped off a lead byte, depth must be reduced
          --current_depth_;
        }
      }
    }

    void ExtractCodePointFromStack() {
      size_t position = transitions_stack_.size() - 1;

      while (!util::Utf8Utils::IsLeadByte(transitions_stack_[position])) {
        --position;
      }

      switch (utf8_length_stack_[position]) {
        case 0:
          codepoint_ = transitions_stack_[position];
          break;
        case 1:
          codepoint_ = ((transitions_stack_[position] & 0x1f) << 6)
              | ((transitions_stack_[position + 1] & 0x3f) << 0);
          break;
        case 2:
          codepoint_ = ((transitions_stack_[position] & 0x0f) << 12)
              | ((transitions_stack_[position + 1] & 0x3f) << 6)
              | ((transitions_stack_[position + 2] & 0x3f) << 0);
          break;
        case 3:
          codepoint_ = ((transitions_stack_[position] & 0x07) << 18)
              | ((transitions_stack_[position + 1] & 0x3f) << 12)
              | ((transitions_stack_[position + 2] & 0x3f) << 6)
              | ((transitions_stack_[position + 3] & 0x3f) << 0);
      }
    }
  };

  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* CODEPOINT_STATE_TRAVERSER_H_ */
