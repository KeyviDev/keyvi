/* * keyvi - A key value store.
 *
 * Copyright 2015, 2016 Hendrik Muhs<hendrik.muhs@gmail.com>,
 *                      Narek Gharibyan<narekgharibyan@gmail.com>
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
 * keyvi_file.h
 *
 *  Created on: November 21, 2016
 *      Author: Narek Gharibyan <narekgharibyan@gmail.com>
 */

#ifndef KEYVI_FILE_H
#define KEYVI_FILE_H

#include <fstream>

#include "dictionary/fsa/internal/constants.h"
#include "dictionary/fsa/internal/serialization_utils.h"

namespace keyvi {
namespace dictionary {

class KeyViFile {
    using ptree=boost::property_tree::ptree;
public:
    explicit KeyViFile(const std::string& filename)
        : file_stream_(filename, std::ios::binary)
    {
        using namespace ::boost;
        using namespace fsa::internal;

        if (!file_stream_.good()) {
            throw std::invalid_argument("file not found");
        }

        char magic[KEYVI_FILE_MAGIC_LEN];
        file_stream_.read(magic, KEYVI_FILE_MAGIC_LEN);
        // check magic
        if (std::strncmp(magic, KEYVI_FILE_MAGIC, KEYVI_FILE_MAGIC_LEN)){
            throw std::invalid_argument("not a keyvi file");
        }

        automata_properties_ = SerializationUtils::ReadJsonRecord(file_stream_);
        persistence_offset_ = file_stream_.tellg();

        if (lexical_cast<int> (automata_properties_.get<std::string>("version")) < KEYVI_FILE_VERSION_MIN) {
          throw std::invalid_argument("this version of keyvi file is unsupported");
        }

        const ptree sparse_array_properties = SerializationUtils::ReadJsonRecord(file_stream_);

        if (lexical_cast<int> (sparse_array_properties.get<std::string>("version")) < KEYVI_FILE_PERSISTENCE_VERSION_MIN) {
          throw std::invalid_argument("this versions of keyvi file is unsupported");
        }

        const bool compact_size = lexical_cast<uint32_t> (sparse_array_properties.get<std::string>("version")) == 2;
        const size_t bucket_size = compact_size ? sizeof(uint16_t) : sizeof(uint32_t);
        const size_t array_size = lexical_cast<size_t>(sparse_array_properties.get<std::string>("size"));

        // check for file truncation
        file_stream_.seekg((size_t)file_stream_.tellg() + array_size + bucket_size * array_size - 1);
        if (file_stream_.peek() == EOF) {
            throw std::invalid_argument("file is corrupt(truncated)");
        }

        file_stream_.get();
        value_store_offset_ = file_stream_.tellg();
    }

    ptree automataProperties() const {
        return automata_properties_;
    }

    std::istream& persistenceStream() {
        return file_stream_.seekg(persistence_offset_);
    }

    std::istream& valueStoreStream() {
        return file_stream_.seekg(value_store_offset_);
    }

private:
    std::ifstream   file_stream_;
    ptree           automata_properties_;
    std::streampos  persistence_offset_;
    std::streampos  value_store_offset_;
};

} /* namespace dictionary */
} /* namespace keyvi */
#endif //KEYVI_FILE_H
