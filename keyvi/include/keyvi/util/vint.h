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
 * vint.h
 *
 * Variable length integer coding
 *
 * based on http://techoverflow.net/blog/2013/01/25/efficiently-encoding-variable-length-integers-in-cc/
 *
 *  Created on: Nov 3, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_VINT_H_
#define KEYVI_UTIL_VINT_H_

#include <string>

namespace keyvi {
namespace util {

/**
 * Encodes an unsigned variable-length integer using the MSB algorithm.
 * @param value The input value. Any standard integer type is allowed.
 * @param output A pointer to a piece of reserved memory. Should have a minimum size dependent on the input size (32 bit
 * = 5 bytes, 64 bit = 10 bytes).
 * @parma output_size_ptr A pointer to a single integer that is set to the number of bytes used in the output memory.
 */
template <typename int_t = uint64_t>
void encodeVarInt(int_t value, uint8_t* output, size_t* output_size_ptr) {
  size_t output_size = 0;
  // While more than 7 bits of data are left, occupy the last output byte
  // and set the next byte flag
  while (value > 127) {
    // |128: Set the next byte flag
    output[output_size] = ((uint8_t)(value & 127)) | 128;

    // Remove the seven bits we just wrote
    value >>= 7;
    output_size++;
  }
  output[output_size++] = ((uint8_t)value) & 127;
  *output_size_ptr = output_size;
}

/**
 * Encodes an unsigned variable-length short using the MSB algorithm.
 * @param value The input value. Any standard integer type is allowed.
 * @param output A pointer to a piece of reserved memory. Should have a minimum size dependent on the input size (32 bit
 * = 5 bytes, 64 bit = 10 bytes).
 * @parma output_size_ptr A pointer to a single integer that is set to the number of bytes used in the output memory.
 */
template <typename int_t = uint64_t>
void encodeVarShort(int_t value, uint16_t* output, size_t* output_size_ptr) {
  size_t output_size = 0;
  // While more than 15 bits of data are left, occupy the last output byte
  // and set the next byte flag
  while (value > 32767) {
    // |32768: Set the next byte flag
    output[output_size] = ((uint16_t)(value & 32767)) | 32768;

    // Remove the 15 bits we just wrote
    value >>= 15;
    output_size++;
  }
  output[output_size++] = ((uint16_t)value) & 32767;
  *output_size_ptr = output_size;
}

/**
 * Get required length of variable int.
 * @param value
 * @return required size
 */
template <typename int_t = uint64_t>
size_t getVarIntLength(int_t value) {
  size_t length = 1;
  while (value > 127) {
    ++length;
    // Remove the seven bits we just checked
    value >>= 7;
  }

  return length;
}

// this specialization is roughly twice as fast as the loop
template <>
inline size_t getVarIntLength(uint64_t value) {
  uint32_t log2value = 63 ^ static_cast<uint32_t>(__builtin_clzll(value | 0x1));
  return static_cast<size_t>((log2value * 9 + 73) / 64);
}

/**
 * Get required length of variable short.
 * @param value
 * @return required size
 */
template <typename int_t = uint64_t>
size_t getVarShortLength(int_t value) {
  return (value < 0x8000) ? 1 : (value < 0x40000000) ? 2 : (value > 0x1fffffffffff) ? 4 : 3;
}

/**
 * Encode variable length integer and write it to the given buffer
 * @param value the integer
 * @param output the buffer to write to
 */
template <typename int_t = uint64_t, typename buffer_t>
void encodeVarInt(int_t value, buffer_t* output, size_t* written_bytes) {
  // While more than 7 bits of data are left, occupy the last output byte
  // and set the next byte flag
  size_t length = 0;
  while (value > 127) {
    // |128: Set the next byte flag
    output->push_back(((uint8_t)(value & 127)) | 128);
    // Remove the seven bits we just wrote
    value >>= 7;
    ++length;
  }
  output->push_back(((uint8_t)value) & 127);
  *written_bytes = ++length;
}

/**
 * Decodes an unsigned variable-length integer using the MSB algorithm.
 * @param value The input value. Any standard integer type is allowed.
 */
template <typename int_t = uint64_t>
int_t decodeVarInt(const uint8_t* input) {
  int_t ret = 0;
  for (uint8_t i = 0;; i++) {
    ret |= (int_t)(input[i] & 127) << (7 * i);

    // If the next-byte flag is set
    if (!(input[i] & 128)) {
      break;
    }
  }
  return ret;
}

/**
 * Decodes an unsigned variable-length short using the MSB algorithm.
 * @param value The input value. Any standard integer type is allowed.
 */
template <typename int_t = uint64_t>
int_t decodeVarShort(const uint16_t* input) {
  int_t ret = 0;
  for (uint8_t i = 0;; i++) {
    ret |= (int_t)(input[i] & 32767) << (15 * i);

    // If the next-byte flag is set
    if (!(input[i] & 32768)) {
      break;
    }
  }
  return ret;
}

inline size_t skipVarInt(const char* input) {
  size_t i = 0;

  for (;; i++) {
    // If the next-byte flag is set
    if (!(input[i] & 128)) {
      break;
    }
  }
  return i + 1;
}

inline std::string decodeVarIntString(const char* input) {
  uint64_t length = 0;
  size_t i = 0;

  for (;; i++) {
    length |= (input[i] & 127) << (7 * i);

    // If the next-byte flag is set
    if (!(input[i] & 128)) {
      break;
    }
  }

  return std::string(input + i + 1, length);
}

inline const char* decodeVarIntString(const char* input, size_t* length_ptr) {
  size_t length = 0;
  size_t i = 0;

  for (;; i++) {
    length |= (input[i] & 127) << (7 * i);

    // If the next-byte flag is set
    if (!(input[i] & 128)) {
      break;
    }
  }

  *length_ptr = length;
  return input + i + 1;
}

} /* namespace util */
} /* namespace keyvi */
#endif  // KEYVI_UTIL_VINT_H_
