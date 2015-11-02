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
 * memory_map_manager.h
 *
 *  Created on: May 8, 2014
 *      Author: hendrik
 */

#ifndef MEMORY_MAP_MANAGER_H_
#define MEMORY_MAP_MANAGER_H_

#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

/***
 * A wrapper around boost memory mapping for writing the buffer in external memory.
 * Chunks the memory across multiple files.
 */
class MemoryMapManager
final {
   public:
    MemoryMapManager(size_t chunk_size, boost::filesystem::path directory,
                     boost::filesystem::path filename_pattern)
        : chunk_size_(chunk_size),
          directory_(directory),
          filename_pattern_(filename_pattern) {
    }

    ~MemoryMapManager() {
      for (auto& m : mappings_) {
        delete m.mapping_;
        delete m.region_;
      }
    }

    /* Using GetAdress to read multiple bytes is unsafe as it might be a buffer overflow
     *
     * This API is to check first whether GetAdress is safe to use.
     */
    bool GetAddressQuickTestOk(size_t offset, size_t length){
      size_t chunk_offset = offset % chunk_size_;

      return (length <= (chunk_size_ - chunk_offset));
    }

    void* GetAddress(size_t offset) {
      size_t chunk_number = offset / chunk_size_;
      size_t chunk_offset = offset % chunk_size_;

      void* chunk_address = GetChunk(chunk_number);

      return ((char*) chunk_address + chunk_offset);
    }

    /* Get a buffer as copy.
     *
     * This API is to be used when GetAdress is not safe to use.
     */
    void GetBuffer(size_t offset, void* buffer, size_t buffer_length) {
      size_t chunk_number = offset / chunk_size_;
      size_t chunk_offset = offset % chunk_size_;

      void* chunk_address = GetChunk(chunk_number);
      void* chunk_address_part2 = GetChunk(chunk_number + 1);

      size_t first_chunk_size = std::min(buffer_length, chunk_size_ - chunk_offset);
      size_t second_chunk_size = buffer_length - first_chunk_size;

      std::memcpy(buffer, (char*) chunk_address + chunk_offset, first_chunk_size);
      std::memcpy((char*) buffer + first_chunk_size, (char*) chunk_address_part2, second_chunk_size);
    }

    /**
     * DEPRECATED: Append to the buffer starting from the given offset.
     * Note: Append(buffer, buffer_length) shall be used instead.
     *
     * @param offset offset at where to append.
     * @param buffer the buffer to append.
     * @param buffer_length the buffer length to append.
     */
    void Append(size_t offset, void* buffer, size_t buffer_length){
      size_t chunk_number = offset / chunk_size_;
      size_t chunk_offset = offset % chunk_size_;

      void* chunk_address = GetChunk(chunk_number);
      size_t first_chunk_size = std::min(buffer_length, chunk_size_ - chunk_offset);
      std::memcpy((char*)chunk_address + chunk_offset, buffer, first_chunk_size);

      // handle overflow
      if (buffer_length != first_chunk_size) {
        void* chunk_address_part2 = GetChunk(chunk_number + 1);
        std::memcpy((char*)chunk_address_part2, (char*)buffer + first_chunk_size, buffer_length - first_chunk_size);
      }

      tail_ = offset + buffer_length;
    }

    /**
     * Append to the buffer at the current tail.
     *
     * @param buffer the buffer to append
     * @param buffer_length the buffer length to append
     */
    void Append(void* buffer, size_t buffer_length){
      size_t chunk_number = tail_ / chunk_size_;
      size_t chunk_offset = tail_ % chunk_size_;

      void* chunk_address = GetChunk(chunk_number);
      size_t first_chunk_size = std::min(buffer_length, chunk_size_ - chunk_offset);
      std::memcpy((char*)chunk_address + chunk_offset, buffer, first_chunk_size);

      // handle overflow
      if (buffer_length != first_chunk_size) {
        void* chunk_address_part2 = GetChunk(chunk_number + 1);
        std::memcpy((char*)chunk_address_part2, (char*)buffer + first_chunk_size, buffer_length - first_chunk_size);
      }

      tail_ += buffer_length;
    }

    void push_back(const char c){
      size_t chunk_number = tail_ / chunk_size_;
      size_t chunk_offset = tail_ % chunk_size_;

      char* chunk_address = static_cast<char*> (GetChunk(chunk_number));
      chunk_address[chunk_offset] = c;
      ++tail_;
    }

    bool Compare(size_t offset, void* buffer, size_t buffer_length){
      size_t chunk_number = offset / chunk_size_;
      size_t chunk_offset = offset % chunk_size_;

      void* chunk_address = GetChunk(chunk_number);
      size_t first_chunk_size = std::min(buffer_length, chunk_size_ - chunk_offset);

      if (std::memcmp((char*)chunk_address + chunk_offset, buffer, first_chunk_size) != 0) {
        return false;
      }

      // no overflow and equal
      if (buffer_length == first_chunk_size) {
        return true;
      }

      // handle overflow
      void* chunk_address_part2 = GetChunk(chunk_number + 1);
      return (std::memcmp((char*)chunk_address_part2, (char*)buffer + first_chunk_size, buffer_length - first_chunk_size) == 0);
    }

    void Write (std::ostream& stream, size_t end) const {
      size_t number_of_chunks = mappings_.size();
      if (number_of_chunks == 0){
        return;
      }

      // write all but the last
      for (size_t i = 0; i< number_of_chunks - 1; ++i){
        char *ptr = (char*) mappings_[i].region_->get_address();
        stream.write (ptr, chunk_size_);
      }
      char *ptr = (char*) mappings_[number_of_chunks - 1].region_->get_address();
        stream.write (ptr, end - ((number_of_chunks - 1) * chunk_size_));
    }

   size_t GetSize() const {
     return tail_;
   }

   private:
    struct mapping {
      boost::interprocess::file_mapping* mapping_;
      boost::interprocess::mapped_region* region_;
    };

    size_t chunk_size_;
    std::vector<mapping> mappings_;
    boost::filesystem::path directory_;
    boost::filesystem::path filename_pattern_;
    size_t tail_ = 0;

    void* GetChunk(size_t chunk_number) {
      while (chunk_number >= mappings_.size()) {
        CreateMapping();
      }

      return mappings_[chunk_number].region_->get_address();
    }

    void CreateMapping() {
      mapping new_mapping;

      boost::filesystem::path filename(directory_);
      filename /= filename_pattern_;
      filename += "_";
      filename += std::to_string(mappings_.size());

      std::filebuf fbuf;
      fbuf.open(
          filename.native().c_str(),
          std::ios_base::in | std::ios_base::out | std::ios_base::trunc
              | std::ios_base::binary);
      // Set the size
      fbuf.pubseekoff(chunk_size_ - 1, std::ios_base::beg);
      fbuf.sputc(0);
      fbuf.close();

      new_mapping.mapping_ = new boost::interprocess::file_mapping(
          filename.native().c_str(), boost::interprocess::read_write);

      new_mapping.region_ = new boost::interprocess::mapped_region(
          *new_mapping.mapping_, boost::interprocess::read_write);

      mappings_.push_back(new_mapping);
    }
  };

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* MEMORY_MAP_MANAGER_H_ */
