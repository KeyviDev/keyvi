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
 * bit_vector.h
 *
 *  Created on: May 2, 2014
 *      Author: hendrik
 */

#ifndef BIT_VECTOR_H_
#define BIT_VECTOR_H_

#include <limits.h>
#include <algorithm>
#include <array>

#if defined (__GNUC__) || defined(__GNUG__)
#include "dictionary/fsa/internal/bit_vector_64.h"
#else

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/**
 * A bitvector implementation with a fixed length and special methods for finding free slots.
 *
 * @tparam size the size of the BitVector.
 */
template<std::size_t Tsize>
struct BitVector
final {
   public:
    template<std::size_t TsizeOther> friend class BitVector;

    BitVector() {
      Clear();
    }

    /**
     * Move Assignment operator.
     * @param other BitVector of the same size/type
     * @return a bitvector instance
     */
    BitVector(BitVector&& other):
      bits_(std::move(other.bits_))
    {}

    BitVector& operator=(const BitVector&& other) {
      bits_= std::move(other.bits_);
      return *this;
    }

    /**
     * Sets a bit in the bitvector.
     * @param bit the bit to set.
     */
    inline void Set(const size_t bit) {
      bits_[bit >> 5] |= (uint32_t) 1 << (bit & 31);
    }

    /**
     * Sets all bits of the given bitvector
     * @param other a bitvector
     * @param start_bit the start position to set the vector
     */
    template<std::size_t TsizeOther>
    inline void  SetVector(const BitVector<TsizeOther>& other, const size_t start_bit) {
      size_t bytePosition = start_bit >> 5;
      size_t bitPosition = start_bit & 31;
      size_t write_length = std::min(bits_.size() - bytePosition, other.bits_.size());

      if (bitPosition == 0)
      {
        for (auto i = 0; i<write_length; i++){
          bits_[bytePosition + i] |= other.bits_[i];
        }
      } else {
        bits_[bytePosition] |= ((uint32_t) other.bits_[0]<< bitPosition);
        for (auto i = 1; i<write_length; i++){
          bits_[bytePosition + i] |= ((other.bits_[i]<< bitPosition) | (other.bits_[i-1] >> (32 - bitPosition)));
        }
        bits_[bytePosition + write_length] |= (other.bits_[write_length-1] >> (32 - bitPosition));
      }
    }

    /**
     * Sets all bits of the given bitvector
     * @param other a bitvector
     * @param start_bit the start position in the given bitvector to use
     */
    template<std::size_t TsizeOther>
    inline void  SetVectorAndShiftOther(const BitVector<TsizeOther>& other, const size_t start_bit_other=0) {
      size_t bytePosition_other = start_bit_other >> 5;
      size_t bitPosition_other = start_bit_other & 31;

      size_t write_length = std::min(bits_.size(), other.bits_.size()) - bytePosition_other;

      for (auto i = 0; i<write_length; i++){
        bits_[i] |= other.GetUnderlyingIntegerAtPosition(bytePosition_other + i, bitPosition_other);
      }
    }


    /**
     * Erase a bit, set it to 0.
     * @param bit the bit to erase.
     */
    inline void Erase(const size_t bit) {
      bits_[bit >> 5] &= (uint32_t) (~(1 << (bit & 31)));
    }

    /** Gets the state of the given bit
     *
     * @param bit the bit
     * @return True if set, false otherwise.
     */
    inline bool Get(const size_t bit) const {
      return (bits_[bit >> 5] & (1 << (bit & 31))) != 0;
    }

    /**
     * Get the next non set bit in the bitvector starting from the given position.
     * @param start_bit the bit to start searching from
     * @return the next unset bit.
     */
    inline int GetNextNonSetBit(size_t start_bit) const {
      size_t bytePosition = start_bit >> 5;
      size_t bitPosition = start_bit & 31;

      uint32_t a = GetUnderlyingIntegerAtPosition(bytePosition, bitPosition);

      while (a == UINT32_MAX) {
        ++bytePosition;
        start_bit += 32;
        a = GetUnderlyingIntegerAtPosition(bytePosition, bitPosition);
      }

      return Position(~a) + start_bit;
    }

    /**
     * Checks whether this bitvector at the given start position and the given bitvector are disjoint.
     * @param other a bitvector to compare with
     * @param start_bit the start position of this bitvector
     * @return true if the sets are disjoint.
     */
    template<std::size_t TsizeOther>
    inline bool Disjoint(const BitVector<TsizeOther>& other, const size_t start_bit) const {
      size_t bytePosition = start_bit >> 5;
      size_t lenthToCheck = std::min(other.bits_.size(),
                                  bits_.size() - bytePosition);
      size_t bitPosition = start_bit & 31;

      for (size_t i = 0; i < lenthToCheck; ++i) {
        uint32_t b = other.bits_[i];
        if (b != 0) {
          uint32_t a = GetUnderlyingIntegerAtPosition(bytePosition, bitPosition);

          if ((a & b) != 0) {
            // shift until it fits
            return false;
          }
        }

        ++bytePosition;
      }

      return true;
    }

    /**
     * Checks whether this bitvector at the given start position and the given bitvector are disjoint and
     * otherwise returns the minimum number of bits the "other" has to be shifted.
     * @param other a bitvector to compare with
     * @param start_bit the start positions
     * @return 0 if the sets are disjoint, otherwise the minimum number of shift operations.
     * @remarks This method is a performance critical.
     */
    template<std::size_t TsizeOther>
    inline int DisjointAndShiftOther(const BitVector<TsizeOther>& other,
                              const size_t start_bit) const {
      size_t byte_position = start_bit >> 5;
      size_t lengthToCheck = std::min(other.bits_.size(),
                                   bits_.size() - byte_position);
      size_t bit_position = start_bit & 31;

      for (size_t i = 0; i < lengthToCheck; ++i) {
        uint32_t b = other.bits_[i];
        if (b != 0) {
          uint32_t a = GetUnderlyingIntegerAtPosition(byte_position, bit_position);

          if ((a & b) != 0) {
            // shift until it fits
            return GetMinimumNumberOfShifts(a, b);
          }
        }

        ++byte_position;
      }

      return 0;
    }

    /**
     * Checks whether this bit vector at the given start position and the given bitvector are disjoint and
     * returns the minimum number of bits to shift until it could fit.
     * @param other the bit vector to compare with
     * @param start_bit the starting position in this bit vector
     * @return 0 if the sets are disjoint, otherwise the minimum number of shift operations.
     * @remark This method is a performance hotspot.
     */
    template<std::size_t TsizeOther>
    inline int DisjointAndShiftThis(const BitVector<TsizeOther>& other,
                             const size_t start_bit) const {
      size_t byte_position = start_bit >> 5;
      size_t lengthToCheck = std::min(other.bits_.size(),
                                   bits_.size() - byte_position);
      size_t bit_position = start_bit & 31;

      for (size_t i = 0; i < lengthToCheck; ++i) {
        uint32_t b = other.bits_[i];
        if (b != 0) {
          uint32_t a = GetUnderlyingIntegerAtPosition(byte_position, bit_position);

          if ((a & b) != 0) {
            // shift until it fits
            return GetMinimumNumberOfShifts(b, a);
          }
        }

        ++byte_position;
      }

      return 0;
    }

    /***
     * Clear the bit vector (reset all to 0)
     */
    void Clear() {
      bits_.fill(0);
    }

    /***
     * Get the size of the bit vector
     */
    size_t Size() const {
      return size_;
    }

#ifndef BITVECTOR_UNIT_TEST
   private:
#endif
    /// internal data
    std::array<uint32_t, (Tsize >> 5) + 1> bits_;
    size_t size_ = Tsize;

// gcc and clang
#if defined (__GNUC__) || defined(__GNUG__)
    inline int Position(uint32_t number) const {
      return __builtin_ffs(number) - 1;
    }
#else
    const int kDeBruijnPositions[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20,
        15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5,
        10, 9 };

    inline int Position(uint32_t number) const {
        uint32_t res = ((uint32_t) (number & -number) * 0x077CB531U)
            >> 27;

        return kDeBruijnPositions[res];
    }
#endif

    inline int GetMinimumNumberOfShifts(uint32_t b, uint32_t a) const {
      int shifts = 1;
      a = a >> 1;
      while ((a & b) != 0) {
        a = a >> 1;
        ++shifts;
      }

      return shifts;
    }

    inline uint32_t GetUnderlyingIntegerAtPosition(const size_t byte_position,
                                                       const size_t bit_position) const {
      if (bit_position == 0) {
        return bits_[byte_position];
      }

      if (byte_position + 1 < bits_.size()) {
        return (bits_[byte_position] >> bit_position)
            | (bits_[byte_position + 1] << (32 - bit_position));
      }

      return bits_[byte_position] >> bit_position;
    }
  };

  } /* namespace internal */
  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */
#endif
#endif /* BIT_VECTOR_H_ */
