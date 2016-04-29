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
 * value_store_persistence.h
 *
 *  Created on: Mar 11, 2016
 *      Author: hendrik
 */

#ifndef VALUE_STORE_PERSISTENCE_H_
#define VALUE_STORE_PERSISTENCE_H_

#include <limits.h>
#include <functional>

#include "dictionary/util/vint.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

template<class HashCodeTypeT = int32_t>
struct RawPointer final {
   public:
    RawPointer() : RawPointer(0, 0, 0) {}

    RawPointer(uint64_t offset, HashCodeTypeT hashcode, size_t length)
        : offset_(offset), hashcode_(hashcode), length_(length) {
      if (length > USHRT_MAX) {
        length_ = USHRT_MAX;
      }
    }

    HashCodeTypeT GetHashcode() const {
      return hashcode_;
    }

    uint64_t GetOffset() const {
      return offset_;
    }

    ushort GetLength() const {
      return length_;
    }

    int GetCookie() const {
      return cookie_;
    }

    void SetCookie(int value) {
      cookie_ = static_cast<ushort>(value);
    }

    bool IsEmpty() const {
      return offset_ == 0 && hashcode_ == 0 && length_ == 0;
    }

    bool operator==(const RawPointer& l) {
      return offset_ == l.offset_;
    }

    static size_t GetMaxCookieSize() {
      return MaxCookieSize;
    }

   private:
    static const size_t MaxCookieSize = 0xFFFF;

    uint64_t offset_;
    HashCodeTypeT hashcode_;
    ushort length_;
    ushort cookie_ = 0;

  };

  template<class PersistenceT, class HashCodeTypeT = int32_t>
  struct RawPointerForCompare final {
   public:
    RawPointerForCompare(const char* value, size_t value_size, PersistenceT* persistence)
        : value_(value), value_size_(value_size), persistence_(persistence) {

      // calculate a hashcode
      HashCodeTypeT h = 31;

      for(size_t i = 0; i < value_size_; ++i) {
        h = (h * 54059) ^ (value[i] * 76963);
      }

      TRACE("hashcode %d", h);

      hashcode_ = h;
    }

    HashCodeTypeT GetHashcode() const {
      return hashcode_;
    }

    bool operator==(const RawPointer<HashCodeTypeT>& l) const {
      TRACE("check equality, 1st hashcode");

      // First filter - check if hash code  is the same
      if (l.GetHashcode() != hashcode_) {
        return false;
      }

      TRACE("check equality, 2nd length");
      size_t length_l = l.GetLength();

      if (length_l < USHRT_MAX) {
        if (length_l != value_size_) {
          return false;
        }

        TRACE("check equality, 3rd buffer %d %d %d", l.GetOffset(), value_size_, util::getVarintLength(length_l));
        // we know the length, skip the length byte and compare the value
        return persistence_->Compare(l.GetOffset() + util::getVarintLength(length_l), (void*) value_, value_size_);
      }

      // we do not know the length, first get it, then compare
      char buf[8];
      persistence_->GetBuffer(l.GetOffset(), buf, 8);

      length_l = util::decodeVarint((uint8_t*)buf);

      TRACE("check equality, 3rd buffer %d %d", l.GetOffset(), value_size_);
      return persistence_->Compare(l.GetOffset() + util::getVarintLength(length_l), (void*) value_, value_size_);
    }

   private:
    const char* value_;
    size_t value_size_;
    PersistenceT* persistence_;
    HashCodeTypeT hashcode_;
  };

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* VALUE_STORE_PERSISTENCE_H_ */
