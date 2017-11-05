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
 * packed_state.h
 *
 *  Created on: Apr 30, 2014
 *      Author: hendrik
 */

#ifndef PACKED_STATE_H_
#define PACKED_STATE_H_

#include <algorithm>
#include <string>

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * Represents a state in the state hashtable. Since we'll need to save millions of these,
 * we aim to make each object very small.
 *
 *  @tparam OffsetTypeT
 *  @tparam HashCodeTypeT
 */
template<class OffsetTypeT = uint32_t, class HashCodeTypeT = int32_t>
struct PackedState
final {
   public:
    PackedState()
        : PackedState(0, 0, 0) {
    }
    PackedState(OffsetTypeT offset, HashCodeTypeT hashcode, int num_outgoing)
        : offset_(offset),
          hashcode_(hashcode),
          num_outgoing_and_cookie_((uint32_t) (num_outgoing & 0x1FF)) {
    }

    HashCodeTypeT GetHashcode() const {
      return hashcode_;
    }

    int GetNumberOfOutgoingTransitions() const {
      return static_cast<int>(this->num_outgoing_and_cookie_ & 0x1FF);
    }

    int GetCookie() const {
      return static_cast<int>((this->num_outgoing_and_cookie_ & 0xFFFFFE00)
          >> 9);
    }

    void SetCookie(int value) {
      this->num_outgoing_and_cookie_ = (this->num_outgoing_and_cookie_
          & 0x1FF) | (uint32_t) (value << 9);
    }

    OffsetTypeT GetOffset() const {
      return offset_;
    }

    static size_t GetMaxCookieSize(){
      return MaxCookieSize;
    }

    bool IsEmpty() const {
      return offset_ == 0 && hashcode_ == 0;
    }

    bool operator==(const PackedState& rhs) const {
      return offset_ == rhs.offset_;
    }

   private:
    static const size_t MaxCookieSize = 0x7FFFFE;

    OffsetTypeT offset_;
    HashCodeTypeT hashcode_;
    uint32_t num_outgoing_and_cookie_;

  }
  // this is __very__ size critical, so disable any padding
  __attribute__ ((packed));

  } /* namespace internal */
  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* PACKED_STATE_H_ */
