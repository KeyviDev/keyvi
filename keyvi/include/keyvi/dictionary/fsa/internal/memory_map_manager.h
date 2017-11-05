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
    MemoryMapManager(const size_t chunk_size, const boost::filesystem::path directory,
                     const boost::filesystem::path filename_pattern)
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
    bool GetAddressQuickTestOk(size_t offset, size_t length) const {
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
    void GetBuffer(const size_t offset, void* buffer, const size_t buffer_length) {
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
     * Append to the buffer at the current tail.
     *
     * @param buffer the buffer to append
     * @param buffer_length the buffer length to append
     */
    void Append(const void* buffer, const size_t buffer_length){
      size_t remaining = buffer_length;
      size_t buffer_offset = 0;

      TRACE("append %ld %ld", buffer_offset, remaining);

      while (remaining > 0) {
        TRACE("next chunk remaining: %ld", remaining);

        size_t chunk_number = tail_ / chunk_size_;
        size_t chunk_offset = tail_ % chunk_size_;
        TRACE ("chunk number: %ld offset %ld", chunk_number, chunk_offset);

        void* chunk_address = GetChunk(chunk_number);
        size_t copy_size = std::min(remaining, chunk_size_ - chunk_offset);
        TRACE ("copy size: %ld", copy_size);

        std::memcpy((char*)chunk_address + chunk_offset, (char*)buffer + buffer_offset, copy_size);

        remaining -= copy_size;
        tail_ += copy_size;
        buffer_offset += copy_size;
      }
    }

    void push_back(const char c){
      size_t chunk_number = tail_ / chunk_size_;
      size_t chunk_offset = tail_ % chunk_size_;

      char* chunk_address = static_cast<char*> (GetChunk(chunk_number));
      chunk_address[chunk_offset] = c;
      ++tail_;
    }

    bool Compare(const size_t offset, const void* buffer, const size_t buffer_length){
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

    void Write (std::ostream& stream, const size_t end) const {
      if (persisted_) {
        for (size_t i =0; i < number_of_chunks_; i++)
        {
          std::ifstream data_file;
          data_file.open (GetFilenameForChunk(i).native().c_str(), std::ios::binary);
          stream << data_file.rdbuf();
          data_file.close();
        }
      } else if (number_of_chunks_ == 0) {
        return;
      }else {
        size_t remaining = end;
        int chunk = 0;

        while (remaining > 0) {
          size_t bytes_in_chunk = std::min(chunk_size_, remaining);
          TRACE("write chunk %d, with size: %ld, remaining: %ld", i, bytes_in_chunk, remaining);

          char *ptr = (char*) mappings_[chunk].region_->get_address();
                    stream.write (ptr, bytes_in_chunk);

          remaining -= bytes_in_chunk;
          ++chunk;
        }
      }
    }

   size_t GetSize() const {
     return tail_;
   }

   /**
    * Frees up all mmap's, should be called after everything has been written.
    */
   void Persist() {
     persisted_ = true;
     for (auto& m : mappings_) {
       m.region_->flush();
       delete m.region_;
       delete m.mapping_;
     }

     // truncate last file according to the written buffers
     if (number_of_chunks_ > 0) {
       boost::filesystem::resize_file(GetFilenameForChunk(number_of_chunks_ - 1), tail_ - ((number_of_chunks_ - 1) * chunk_size_));
     }

     mappings_.clear();
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
    bool persisted_ = false;
    size_t number_of_chunks_ = 0;

    boost::filesystem::path GetFilenameForChunk(int i) const {
      boost::filesystem::path filename(directory_);
      filename /= filename_pattern_;
      filename += "_";
      filename += std::to_string(i);
      return filename;
    }

    void* GetChunk(const size_t chunk_number) {
      while (chunk_number >= number_of_chunks_) {
        CreateMapping();
      }

      return mappings_[chunk_number].region_->get_address();
    }

    void CreateMapping() {
      TRACE("create new mapping %d", number_of_chunks_ + 1);
      mapping new_mapping;

      boost::filesystem::path filename = GetFilenameForChunk(number_of_chunks_);

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

      // prevent pre-fetching pages by the OS which does not make sense as values usually fit into few pages
      new_mapping.region_->advise(boost::interprocess::mapped_region::advice_types::advice_random);

      mappings_.push_back(new_mapping);
      ++number_of_chunks_;
    }
  };

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* MEMORY_MAP_MANAGER_H_ */
