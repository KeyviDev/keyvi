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
 * automata.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_FSA_AUTOMATA_H_
#define KEYVI_DICTIONARY_FSA_AUTOMATA_H_

#include <memory>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "keyvi/dictionary/dictionary_merger_fwd.h"
#include "keyvi/dictionary/dictionary_properties.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/dictionary/fsa/internal/intrinsics.h"
#include "keyvi/dictionary/fsa/internal/memory_map_flags.h"
#include "keyvi/dictionary/fsa/internal/value_store_factory.h"
#include "keyvi/dictionary/fsa/traversal/traversal_base.h"
#include "keyvi/dictionary/fsa/traversal/weighted_traversal.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {

/**
 * Lookup table to find outgoing transitions quickly(in parallel) by xor'ing the
 * real buffer with this table.
 * (also used for the intrinsic(SSE 4.2) based implementation)
 */
static unsigned char OUTGOING_TRANSITIONS_MASK[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
    0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e,
    0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71,
    0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84,
    0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
    0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
    0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
    0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

/// TODO: refactor (split) class Automata, so there is no need for param "loadVS" and friend classes
class Automata final {
 public:
  explicit Automata(const std::string& file_name,
                    loading_strategy_types loading_strategy = loading_strategy_types::lazy)
      : Automata(std::make_shared<DictionaryProperties>(DictionaryProperties::FromFile(file_name)), loading_strategy,
                 true) {}

 private:
  explicit Automata(const dictionary_properties_t& dictionary_properties, loading_strategy_types loading_strategy,
                    const bool load_value_store)
      : dictionary_properties_(dictionary_properties) {
    file_mapping_ = boost::interprocess::file_mapping(dictionary_properties_->GetFileName().c_str(),
                                                      boost::interprocess::read_only);

    const boost::interprocess::map_options_t map_options =
        internal::MemoryMapFlags::FSAGetMemoryMapOptions(loading_strategy);

    TRACE("labels start offset: %d", dictionary_properties_.GetPersistenceOffset());
    labels_region_ = boost::interprocess::mapped_region(file_mapping_, boost::interprocess::read_only,
                                                        dictionary_properties_->GetPersistenceOffset(),
                                                        dictionary_properties_->GetSparseArraySize(), 0, map_options);

    TRACE("transitions start offset: %d", dictionary_properties_.GetTransitionsOffset());
    transitions_region_ = boost::interprocess::mapped_region(
        file_mapping_, boost::interprocess::read_only, dictionary_properties_->GetTransitionsOffset(),
        dictionary_properties_->GetTransitionsSize(), 0, map_options);

    const auto advise = internal::MemoryMapFlags::FSAGetMemoryMapAdvices(loading_strategy);

    labels_region_.advise(advise);
    transitions_region_.advise(advise);

    labels_ = static_cast<unsigned char*>(labels_region_.get_address());
    transitions_compact_ = static_cast<uint16_t*>(transitions_region_.get_address());

    if (load_value_store) {
      value_store_reader_.reset(
          internal::ValueStoreFactory::MakeReader(dictionary_properties_->GetValueStoreType(), &file_mapping_,
                                                  dictionary_properties_->GetValueStoreProperties(), loading_strategy));
    }
  }

 public:
  Automata& operator=(Automata const&) = delete;
  Automata(const Automata& that) = delete;

  /**
   * Get the start(root) stage of the FSA
   *
   * @return index of root state.
   */
  uint64_t GetStartState() const { return dictionary_properties_->GetStartState(); }

  uint64_t GetNumberOfKeys() const { return dictionary_properties_->GetNumberOfKeys(); }

  bool Empty() const { return 0 == GetNumberOfKeys(); }

  size_t SparseArraySize() const { return dictionary_properties_->GetSparseArraySize(); }

  internal::value_store_t GetValueStoreType() const { return dictionary_properties_->GetValueStoreType(); }

  uint64_t TryWalkTransition(uint64_t starting_state, unsigned char c) const {
    if (labels_[starting_state + c] == c) {
      return ResolvePointer(starting_state, c);
    }
    return 0;
  }

  /**
   * Get the outgoing states of state quickly in 1 step.
   *
   * @param starting_state The state
   * @param outgoing_states a vector given by reference to put n the outgoing states
   * @param outgoing_symbols a vector given by reference to put in the outgoing symbols (labels)
   *
   * todo: rewrite to avoid push_back on vectors, see:
   * http://lemire.me/blog/2012/06/20/do-not-waste-time-with-stl-vectors/
   */
  template <class TransitionT, typename std::enable_if<std::is_base_of<traversal::Transition, TransitionT>::value,
                                                       traversal::Transition>::type* = nullptr>
  void GetOutGoingTransitions(uint64_t starting_state, traversal::TraversalState<TransitionT>* traversal_state,
                              traversal::TraversalPayload<TransitionT>* payload) const {
    // reset the state
    traversal_state->Clear();

#if defined(KEYVI_SSE42)
    // Optimized version using SSE4.2, see http://www.strchr.com/strcmp_and_strlen_using_sse_4.2

    __m128i* labels_as_m128 = reinterpret_cast<__m128i*>(labels_ + starting_state);
    __m128i* mask_as_m128 = reinterpret_cast<__m128i*>(OUTGOING_TRANSITIONS_MASK);
    unsigned char symbol = 0;

    // check 16 bytes at a time
    for (int offset = 0; offset < 16; ++offset) {
      __m128i mask =
          _mm_cmpestrm(_mm_loadu_si128(labels_as_m128), 16, _mm_loadu_si128(mask_as_m128), 16,
                       _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_MASKED_POSITIVE_POLARITY | _SIDD_BIT_MASK);

      uint64_t mask_int = _mm_extract_epi64(mask, 0);
      TRACE("Bitmask %d", mask_int);

      if (mask_int != 0) {
        for (auto i = 0; i < 16; ++i) {
          if ((mask_int & 1) == 1) {
            TRACE("push symbol+%d", symbol + i);
            traversal_state->Add(ResolvePointer(starting_state, symbol + i), symbol + i, payload);
          }
          mask_int = mask_int >> 1;
        }
      }

      ++labels_as_m128;
      ++mask_as_m128;
      symbol += 16;
    }
#else
    uint64_t* labels_as_ll = reinterpret_cast<uint64_t*>(labels_ + starting_state);
    uint64_t* mask_as_ll = reinterpret_cast<uint64_t*>(OUTGOING_TRANSITIONS_MASK);
    unsigned char symbol = 0;

    // check 8 bytes at a time
    for (int offset = 0; offset < 32; ++offset) {
      uint64_t xor_labels_with_mask = *labels_as_ll ^ *mask_as_ll;

      if (((xor_labels_with_mask & 0x00000000000000ffULL) == 0)) {
        traversal_state->Add(ResolvePointer(starting_state, symbol), symbol, payload);
      }
      if ((xor_labels_with_mask & 0x000000000000ff00ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 1), symbol + 1, payload);
      }
      if ((xor_labels_with_mask & 0x0000000000ff0000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 2), symbol + 2, payload);
      }
      if ((xor_labels_with_mask & 0x00000000ff000000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 3), symbol + 3, payload);
      }
      if ((xor_labels_with_mask & 0x000000ff00000000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 4), symbol + 4, payload);
      }
      if ((xor_labels_with_mask & 0x0000ff0000000000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 5), symbol + 5, payload);
      }
      if ((xor_labels_with_mask & 0x00ff000000000000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 6), symbol + 6, payload);
      }
      if ((xor_labels_with_mask & 0xff00000000000000ULL) == 0) {
        traversal_state->Add(ResolvePointer(starting_state, symbol + 7), symbol + 7, payload);
      }

      ++labels_as_ll;
      ++mask_as_ll;
      symbol += 8;
    }
#endif

    // post, e.g. sort transitions
    TRACE("postprocess transitions");
    traversal_state->PostProcess(payload);

    return;
  }

  template <class TransitionT,
            typename std::enable_if<std::is_base_of<traversal::WeightedTransition, TransitionT>::value,
                                    traversal::WeightedTransition>::type* = nullptr>
  inline void GetOutGoingTransitions(uint64_t starting_state, traversal::TraversalState<TransitionT>* traversal_state,
                                     traversal::TraversalPayload<TransitionT>* payload) const {
    // reset the state
    traversal_state->Clear();
    uint32_t parent_weight = GetWeightValue(starting_state);

#if defined(KEYVI_SSE42)
    // Optimized version using SSE4.2, see http://www.strchr.com/strcmp_and_strlen_using_sse_4.2

    __m128i* labels_as_m128 = reinterpret_cast<__m128i*>(labels_ + starting_state);
    __m128i* mask_as_m128 = reinterpret_cast<__m128i*>(OUTGOING_TRANSITIONS_MASK);
    unsigned char symbol = 0;

    // check 16 bytes at a time
    for (int offset = 0; offset < 16; ++offset) {
      __m128i mask =
          _mm_cmpestrm(_mm_loadu_si128(labels_as_m128), 16, _mm_loadu_si128(mask_as_m128), 16,
                       _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_MASKED_POSITIVE_POLARITY | _SIDD_BIT_MASK);

      uint64_t mask_int = _mm_extract_epi64(mask, 0);
      TRACE("Bitmask %d", mask_int);

      if (mask_int != 0) {
        for (auto i = 0; i < 16; ++i) {
          if ((mask_int & 1) == 1) {
            TRACE("push symbol+%d", symbol + i);
            uint64_t child_state = ResolvePointer(starting_state, symbol + i);
            uint32_t weight = GetWeightValue(child_state);
            weight = weight != 0 ? weight : parent_weight;
            traversal_state->Add(child_state, weight, symbol + i, payload);
          }
          mask_int = mask_int >> 1;
        }
      }

      ++labels_as_m128;
      ++mask_as_m128;
      symbol += 16;
    }
#else
    uint64_t* labels_as_ll = reinterpret_cast<uint64_t*>(labels_ + starting_state);
    uint64_t* mask_as_ll = reinterpret_cast<uint64_t*>(OUTGOING_TRANSITIONS_MASK);
    unsigned char symbol = 0;

    // check 8 bytes at a time
    for (int offset = 0; offset < 32; ++offset) {
      uint64_t xor_labels_with_mask = *labels_as_ll ^ *mask_as_ll;

      if (((xor_labels_with_mask & 0x00000000000000ffULL) == 0)) {
        uint64_t child_state = ResolvePointer(starting_state, symbol);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol, payload);
      }
      if ((xor_labels_with_mask & 0x000000000000ff00ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 1);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 1, payload);
      }
      if ((xor_labels_with_mask & 0x0000000000ff0000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 2);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 2, payload);
      }
      if ((xor_labels_with_mask & 0x00000000ff000000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 3);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 3, payload);
      }
      if ((xor_labels_with_mask & 0x000000ff00000000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 4);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 4, payload);
      }
      if ((xor_labels_with_mask & 0x0000ff0000000000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 5);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 5, payload);
      }
      if ((xor_labels_with_mask & 0x00ff000000000000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 6);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 6, payload);
      }
      if ((xor_labels_with_mask & 0xff00000000000000ULL) == 0) {
        uint64_t child_state = ResolvePointer(starting_state, symbol + 7);
        uint32_t weight = GetWeightValue(child_state);
        weight = weight != 0 ? weight : parent_weight;
        traversal_state->Add(child_state, weight, symbol + 7, payload);
      }

      ++labels_as_ll;
      ++mask_as_ll;
      symbol += 8;
    }
#endif

    // post, e.g. sort transitions
    TRACE("postprocess transitions");
    traversal_state->PostProcess(payload);

    return;
  }

  bool IsFinalState(uint64_t state_to_check) const {
    if (labels_[state_to_check + FINAL_OFFSET_TRANSITION] == FINAL_OFFSET_CODE) {
      return true;
    }
    return false;
  }

  uint64_t GetStateValue(uint64_t state) const {
    return keyvi::util::decodeVarShort(transitions_compact_ + state + FINAL_OFFSET_TRANSITION);
  }

  uint32_t GetWeightValue(uint64_t state) const {
    if (labels_[state + INNER_WEIGHT_TRANSITION_COMPACT] != 0) {
      return 0;
    }

    return (transitions_compact_[state + INNER_WEIGHT_TRANSITION_COMPACT]);
  }

  internal::IValueStoreReader::attributes_t GetValueAsAttributeVector(uint64_t state_value) const {
    assert(value_store_reader_);
    return value_store_reader_->GetValueAsAttributeVector(state_value);
  }

  std::string GetValueAsString(uint64_t state_value) const {
    assert(value_store_reader_);
    return value_store_reader_->GetValueAsString(state_value);
  }

  std::string GetRawValueAsString(uint64_t state_value) const {
    assert(value_store_reader_);
    return value_store_reader_->GetRawValueAsString(state_value);
  }

  std::string GetStatistics() const { return dictionary_properties_->GetStatistics(); }

  std::string GetManifest() const { return dictionary_properties_->GetManifest(); }

 private:
  dictionary_properties_t dictionary_properties_;
  std::unique_ptr<internal::IValueStoreReader> value_store_reader_;
  boost::interprocess::file_mapping file_mapping_;
  boost::interprocess::mapped_region labels_region_;
  boost::interprocess::mapped_region transitions_region_;
  unsigned char* labels_;
  uint16_t* transitions_compact_;

  template <keyvi::dictionary::fsa::internal::value_store_t>
  friend class keyvi::dictionary::DictionaryMerger;

  internal::IValueStoreReader* GetValueStore() const {
    assert(value_store_reader_);
    return value_store_reader_.get();
  }

  inline uint64_t ResolvePointer(uint64_t starting_state, unsigned char c) const {
    uint16_t pt = le16toh(transitions_compact_[starting_state + c]);
    uint64_t resolved_ptr;

    if ((pt & 0xC000) == 0xC000) {
      TRACE("Compact Transition uint16 absolute");

      resolved_ptr = pt & 0x3FFF;
      TRACE("Compact Transition after resolve %d", resolved_ptr);
      return resolved_ptr;
    }

    if (pt & 0x8000) {
      TRACE("Compact Transition overflow %d (from %d) starting from %d", pt, starting_state + c, starting_state);

      // clear the first bit
      pt &= 0x7FFF;
      size_t overflow_bucket;
      TRACE("Compact Transition overflow bucket %d", pt);

      overflow_bucket = (pt >> 4) + starting_state + c - COMPACT_SIZE_WINDOW;

      TRACE("Compact Transition found overflow bucket %d", overflow_bucket);

      resolved_ptr = keyvi::util::decodeVarShort(transitions_compact_ + overflow_bucket);
      resolved_ptr = (resolved_ptr << 3) + (pt & 0x7);

      if (pt & 0x8) {
        // relative coding
        resolved_ptr = (starting_state + c) - resolved_ptr + COMPACT_SIZE_WINDOW;
      }

    } else {
      TRACE("Compact Transition uint16 transition %d", pt);
      resolved_ptr = (starting_state + c) - pt + COMPACT_SIZE_WINDOW;
    }

    TRACE("Compact Transition after resolve %d", resolved_ptr);
    return resolved_ptr;
  }
};

// shared pointer
typedef std::shared_ptr<const Automata> automata_t;

} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_FSA_AUTOMATA_H_
