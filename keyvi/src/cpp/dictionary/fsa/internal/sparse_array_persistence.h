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
 * sparse_array_persistence.h
 *
 *  Created on: May 5, 2014
 *      Author: hendrik
 */

#ifndef SPARSE_ARRAY_PERSISTENCE_H_
#define SPARSE_ARRAY_PERSISTENCE_H_

#include <cstring>
#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/memory_map_manager.h"
#include "dictionary/util/vint.h"
#include "dictionary/util/endian.h"

// #define PERSISTENCE_DEBUG
#ifdef PERSISTENCE_DEBUG
#include <assert.h>
#endif

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

template<class BucketT = uint16_t>
class SparseArrayPersistence
final {
   public:
    SparseArrayPersistence(size_t memory_limit,
                           boost::filesystem::path temporary_path)
        : in_memory_buffer_offset_(0) {
      buffer_size_ = memory_limit / (sizeof(unsigned char) + sizeof(BucketT));

      // align it to 16bit (for fast memcpy)
      buffer_size_ += 16 - (buffer_size_ % 16);
      flush_size_ = (buffer_size_ * 3) / 5;
      // align it to 16bit
      flush_size_ += 16 - (flush_size_ % 16);

      TRACE("Memory Limit: %d buffer size: %d flush size: %d", memory_limit, buffer_size_, flush_size_);

      labels_ = new unsigned char[buffer_size_];
      std::memset(labels_, 0, buffer_size_);

      temporary_directory_ = temporary_path;
      temporary_directory_ /= boost::filesystem::unique_path(
          "dictionary-fsa-%%%%-%%%%-%%%%-%%%%");
      boost::filesystem::create_directory(temporary_directory_);

      // size of external memory chunk: not more than 1 or 4 GB
      size_t external_memory_chunk_size = std::min(flush_size_ * 10, (size_t)1073741824);

      // the chunk size must be a multiplier of the flush_size
      external_memory_chunk_size = external_memory_chunk_size - (external_memory_chunk_size % flush_size_);

      labels_extern_ = new MemoryMapManager(external_memory_chunk_size,
                                            temporary_directory_,
                                            "characterTableFileBuffer");

      transitions_ = new BucketT[buffer_size_];
      std::memset(transitions_, 0, buffer_size_ * sizeof(BucketT));

      transitions_extern_ = new MemoryMapManager(external_memory_chunk_size * sizeof(BucketT),
                                                 temporary_directory_,
                                                 "valueTableFileBuffer");
    }

    ~SparseArrayPersistence() {
      delete labels_extern_;
      delete transitions_extern_;
      if (labels_) {
        delete[] labels_;
        delete[] transitions_;
      }
      boost::filesystem::remove_all(temporary_directory_);
    }

    SparseArrayPersistence() = delete;
    SparseArrayPersistence& operator=(SparseArrayPersistence const&) = delete;
    SparseArrayPersistence(const SparseArrayPersistence& that) = delete;

    void BeginNewState(size_t offset) {
      if ((offset + COMPACT_SIZE_WINDOW + NUMBER_OF_STATE_CODINGS)
          >= (buffer_size_ + in_memory_buffer_offset_)) {
        FlushBuffers();
      }

      if (offset > highest_state_begin_) {
        highest_state_begin_ = offset;
        TRACE("new highest state: %d", offset);
      }
    }

    void WriteTransition(size_t offset, unsigned char transitionId,
                         BucketT transitionPointer) {

      TRACE("write transition at %d with %d/%d", offset, transitionId, transitionPointer);
      if (offset >= in_memory_buffer_offset_) {
#ifdef PERSISTENCE_DEBUG
        assert (labels_[offset - in_memory_buffer_offset_] == 0);
        assert (transitions_[offset - in_memory_buffer_offset_] == 0);
#endif

        labels_[offset - in_memory_buffer_offset_] = transitionId;
        transitions_[offset - in_memory_buffer_offset_] = transitionPointer;
        return;
      }

      unsigned char* label_ptr = (unsigned char*) labels_extern_->GetAddress(offset);
      *label_ptr = transitionId;

      BucketT* transition_ptr = (BucketT*) transitions_extern_->GetAddress(
                offset * sizeof(BucketT));
      *transition_ptr = HostOrderToPesistenceOrder(transitionPointer);
    }

    void WriteRawValue(size_t offset, void* buffer, size_t length) {
#ifdef PERSISTENCE_DEBUG
        assert (offset + length < in_memory_buffer_offset_+ buffer_size_);

        assert (transitions_[offset - in_memory_buffer_offset_] == 0);

        for (size_t i; i< length/2; ++i){
          assert (transitions_[offset - in_memory_buffer_offset_ + i] == 0);
        }
#endif

        // todo: this is rather pointless: 99% are small copies not worth the call overhead
        memcpy(transitions_ + offset - in_memory_buffer_offset_, buffer, length);

        // persist the highest position we write into
        highest_raw_write_bucket_ = std::max(highest_raw_write_bucket_, offset + length);
      return;
    }

    int ReadTransitionLabel(size_t offset) const {
      if (offset >= in_memory_buffer_offset_) {
        return labels_[offset - in_memory_buffer_offset_];
      }

      unsigned char* ptr = (unsigned char*) labels_extern_->GetAddress(offset);
      return *ptr;
    }

    BucketT ReadTransitionValue(size_t offset) const {
      if (offset >= in_memory_buffer_offset_) {
        return transitions_[offset - in_memory_buffer_offset_];
      }

      BucketT* ptr = (BucketT*) transitions_extern_->GetAddress(
          offset * sizeof(BucketT));

      return PersistenceOrderToHostOrder(*ptr);
    }

    uint64_t ResolveTransitionValue(size_t offset, BucketT value) const;

    uint64_t ReadFinalValue(size_t offset) const;

    /**
     * Flush buffer up to the given position
     * @param position the latest position until flush should happen
     */
    void Flush() {
      // make idempotent, so it can be called twice or more);
      if (labels_) {
        size_t highest_write_position = std::max(highest_state_begin_ + MAX_TRANSITIONS_OF_A_STATE, highest_raw_write_bucket_);

        labels_extern_->Append(labels_,
                                    (highest_write_position - in_memory_buffer_offset_));

        // in place re-write
        HostOrderToPersistenceOrder(transitions_, highest_write_position - in_memory_buffer_offset_);

        transitions_extern_->Append(transitions_,
                                    (highest_write_position - in_memory_buffer_offset_) * sizeof(BucketT));

        delete[] labels_;
        delete[] transitions_;
        labels_ = 0;
        transitions_ = 0;
      }
    }

    void Write(std::ostream& stream) {
      boost::property_tree::ptree pt;
      pt.put("version", GetVersion());

      size_t highest_write_position = std::max(highest_state_begin_ + MAX_TRANSITIONS_OF_A_STATE, highest_raw_write_bucket_);

      pt.put("size",
             std::to_string(highest_write_position));

      internal::SerializationUtils::WriteJsonRecord(stream, pt);
      TRACE("Wrote JSON header, stream at %d", stream.tellp());

      labels_extern_->Write(stream,
                            highest_write_position);

      TRACE("Wrote Labels, stream at %d", stream.tellp());

      transitions_extern_->Write(
          stream, (highest_write_position) * sizeof(BucketT));
      TRACE("Wrote Transitions, stream at %d", stream.tellp());
    }

   private:
    unsigned char* labels_;
    MemoryMapManager* labels_extern_;
    BucketT* transitions_;
    MemoryMapManager* transitions_extern_;
    boost::filesystem::path temporary_directory_;

    size_t in_memory_buffer_offset_;
    size_t buffer_size_;
    size_t flush_size_;
    size_t highest_state_begin_ = 0;
    size_t highest_raw_write_bucket_ = 0;

    inline void FlushBuffers() {
      labels_extern_->Append(labels_,
                                  flush_size_);

      TRACE ("Write labels from %d to %d (flushsize %d)", in_memory_buffer_offset_, in_memory_buffer_offset_ + flush_size_, flush_size_);

      // in place re-write
      HostOrderToPersistenceOrder(transitions_, flush_size_);

      transitions_extern_->Append(transitions_,
                                  flush_size_ * sizeof(BucketT));

      size_t overlap = buffer_size_ - flush_size_;

      std::memcpy(labels_, labels_ + flush_size_, overlap);
      std::memcpy(transitions_, transitions_ + flush_size_,
                  sizeof(BucketT) * overlap);

      std::memset(labels_ + overlap, 0, flush_size_);
      std::memset(transitions_ + overlap, 0, sizeof(BucketT) * flush_size_);

      in_memory_buffer_offset_ += flush_size_;
    }

    uint32_t GetVersion() const;

    BucketT PersistenceOrderToHostOrder (BucketT value) const;
    BucketT HostOrderToPesistenceOrder (BucketT value) const;

    void HostOrderToPersistenceOrder(BucketT* values, size_t length) const;

  };

  template<>
  inline uint32_t SparseArrayPersistence<uint32_t>::GetVersion() const {
    return 1;
  }

  template<>
  inline uint32_t SparseArrayPersistence<uint16_t>::GetVersion() const {
    return 2;
  }

  template<>
  inline uint32_t SparseArrayPersistence<uint32_t>::PersistenceOrderToHostOrder(uint32_t value) const {
    return be32toh(value);
  }

  template<>
  inline uint16_t SparseArrayPersistence<uint16_t>::PersistenceOrderToHostOrder(uint16_t value) const {
    return le16toh(value);
  }

  template<>
  inline uint32_t SparseArrayPersistence<uint32_t>::HostOrderToPesistenceOrder(uint32_t value) const {
    return htobe32(value);
  }

  template<>
  inline uint16_t SparseArrayPersistence<uint16_t>::HostOrderToPesistenceOrder(uint16_t value) const {
    return htole16(value);
  }

  template<>
  inline void SparseArrayPersistence<uint32_t>::HostOrderToPersistenceOrder(uint32_t* values, size_t length) const {
    for (size_t i = 0; i < flush_size_; ++i) {
            values[i] = htobe32(values[i]);
    }
  }

  template<>
  inline void SparseArrayPersistence<uint16_t>::HostOrderToPersistenceOrder(uint16_t* values, size_t length) const {
#ifdef CLQ_BIG_ENDIAN
    for (size_t i = 0; i < flush_size_; ++i) {
            values[i] = htole16(values[i]);
    }
#endif
  }


  template<>
  inline uint64_t SparseArrayPersistence<uint32_t>::ResolveTransitionValue(size_t offset, uint32_t value) const {
    return value;
  }

  template<>
  inline uint64_t SparseArrayPersistence<uint16_t>::ResolveTransitionValue(size_t offset, uint16_t value) const {
    uint16_t pt = value;
    uint64_t resolved_ptr;

    if ((pt & 0xC000) == 0xC000) {
      //TRACE("Compact Transition uint16 absolute");

      resolved_ptr = pt & 0x3FFF;
      //TRACE("Compact Transition after resolve %d", resolved_ptr);
      return resolved_ptr;
    }

    if (pt & 0x8000){
      // clear the first bit
      pt &= 0x7FFF;
      size_t overflow_bucket;

      overflow_bucket = (pt >> 4) + offset - 512;

      if (overflow_bucket >= in_memory_buffer_offset_) {
        resolved_ptr = util::decodeVarshort(transitions_ -in_memory_buffer_offset_ + overflow_bucket);
      } else {
        if (transitions_extern_->GetAddressQuickTestOk(overflow_bucket * sizeof(uint16_t), 5)) {
          resolved_ptr = util::decodeVarshort((uint16_t*) transitions_extern_->GetAddress(overflow_bucket * sizeof(uint16_t)));
        } else {
          // value might be on the chunk border, take a secure approach
          uint16_t buffer[10];
          transitions_extern_->GetBuffer((offset + FINAL_OFFSET_TRANSITION) * sizeof(uint16_t), buffer, 10 * sizeof(uint16_t));

          resolved_ptr = util::decodeVarshort(buffer);
        }
      }

      resolved_ptr = (resolved_ptr << 3) + (pt & 0x7);

      if (pt & 0x8){
        // relative coding
        resolved_ptr = offset - resolved_ptr + 512;
      }

    } else {
      resolved_ptr = offset -pt + 512;
    }

    return resolved_ptr;
  }

  template<>
  inline uint64_t SparseArrayPersistence<uint32_t>::ReadFinalValue(size_t offset) const {
    return ReadTransitionValue(offset + FINAL_OFFSET_TRANSITION);
  }

  template<>
  inline uint64_t SparseArrayPersistence<uint16_t>::ReadFinalValue(size_t offset) const {
    if (offset + FINAL_OFFSET_TRANSITION >= in_memory_buffer_offset_) {

      return util::decodeVarshort(transitions_ + offset - in_memory_buffer_offset_ + FINAL_OFFSET_TRANSITION);
    }

    if (transitions_extern_->GetAddressQuickTestOk((offset + FINAL_OFFSET_TRANSITION) * sizeof(uint16_t), 5)) {
      uint16_t* ptr = (uint16_t*) transitions_extern_->GetAddress((offset + FINAL_OFFSET_TRANSITION) * sizeof(uint16_t));
      return util::decodeVarshort(ptr);
    }

    // value might be on the chunk border, take a secure approach
    uint16_t buffer[10];
    transitions_extern_->GetBuffer((offset + FINAL_OFFSET_TRANSITION) * sizeof(uint16_t), buffer, 10 * sizeof(uint16_t));

    return util::decodeVarshort(buffer);
  }

  } /* namespace internal */
  } /* namespace fsa */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* SPARSE_ARRAY_PERSISTENCE_H_ */
