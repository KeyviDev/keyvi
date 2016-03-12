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
 * json_value_store_deprecated.h
 *
 *  Created on: Jul 16, 2014
 *      Author: hendrik
 */

#ifndef JSON_VALUE_STORE_DEPRECATED_H_
#define JSON_VALUE_STORE_DEPRECATED_H_

#include <boost/functional/hash.hpp>
#include <zlib.h>

#include "dictionary/fsa/internal/intrinsics.h"

#if defined(KEYVI_SSE42)
#define RAPIDJSON_SSE42
#endif

#include <boost/lexical_cast.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "dictionary/fsa/internal/ivalue_store.h"
#include "dictionary/fsa/internal/serialization_utils.h"
#include "dictionary/fsa/internal/lru_generation_cache.h"
#include "dictionary/fsa/internal/memory_map_manager.h"
#include "dictionary/util/vint.h"

#include "msgpack.hpp"
// from 3rdparty/xchange: msgpack <-> rapidjson converter
#include "msgpack/type/rapidjson.hpp"
#include "msgpack/zbuffer.hpp"

#include "dictionary/fsa/internal/constants.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

      class JsonValueStoreDeprecatedReader final: public IValueStoreReader {
       public:
        using IValueStoreReader::IValueStoreReader;

        JsonValueStoreDeprecatedReader(std::istream& stream,
                                       boost::interprocess::file_mapping* file_mapping,
                                       bool load_lazy = false)
            : IValueStoreReader(stream, file_mapping) {

          TRACE("JsonValueStoreDeprecatedReader construct");

          properties_ =
              internal::SerializationUtils::ReadJsonRecord(stream);

          size_t offset = stream.tellg();
          size_t strings_size = boost::lexical_cast<size_t>(properties_.get<std::string>("size"));

          // check for file truncation
          if (strings_size > 0) {
            stream.seekg(strings_size - 1, stream.cur);
            if (stream.peek() == EOF) {
              throw std::invalid_argument("file is corrupt(truncated)");
            }
          }

          boost::interprocess::map_options_t map_options = boost::interprocess::default_map_options;

#ifdef MAP_HUGETLB
          map_options |= MAP_HUGETLB;
#endif

          if (!load_lazy) {
#ifdef MAP_POPULATE
            map_options |= MAP_POPULATE;
#endif
          }

          strings_region_ = new boost::interprocess::mapped_region(
              *file_mapping, boost::interprocess::read_only, offset,
              strings_size, 0, map_options);

          strings_ = (const char*) strings_region_->get_address();
        }

        ~JsonValueStoreDeprecatedReader() {
          delete strings_region_;
        }

        virtual value_store_t GetValueStoreType() const override {
          return JSON_VALUE_STORE_DEPRECATED;
        }

        virtual attributes_t GetValueAsAttributeVector(uint64_t fsa_value) const override {
          attributes_t attributes(new attributes_raw_t());

          std::string raw_value = util::decodeVarintString(strings_ + fsa_value);

          //auto length = util::decodeVarint((uint8_t*) strings_ + fsa_value);
          //std::string raw_value(strings_ + fsa_value, length);

          (*attributes)["value"] = raw_value;
          return attributes;
        }

        virtual std::string GetRawValueAsString(uint64_t fsa_value) const override {
          return util::decodeVarintString(strings_ + fsa_value);
        }

        virtual std::string GetValueAsString(uint64_t fsa_value) const override {
          TRACE("JsonValueStoreDeprecatedReader GetValueAsString");
          std::string packed_string = util::decodeVarintString(strings_ + fsa_value);

          if (packed_string[0] == ' ') {
            TRACE("unpack zlib compressed string");
            packed_string = decompress_string(packed_string);

            TRACE("unpacking %s", packed_string.c_str());
          }

          msgpack::unpacked doc;
          msgpack::unpack(&doc, packed_string.data(), packed_string.size());
          rapidjson::Document json_document;

          doc.get().convert(&json_document);

          rapidjson::StringBuffer buffer;
          rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
          json_document.Accept(writer);
          return buffer.GetString();
        }

        virtual std::string GetStatistics() const override {
          std::ostringstream buf;
          boost::property_tree::write_json (buf, properties_, false);
          return buf.str();
        }

       private:
        boost::interprocess::mapped_region* strings_region_;
        const char* strings_;
        boost::property_tree::ptree properties_;

        /** Decompress an STL string using zlib and return the original data. */
        std::string decompress_string(const std::string& str) const
        {
            z_stream zs;                        // z_stream is zlib's control structure
            memset(&zs, 0, sizeof(zs));

            if (inflateInit(&zs) != Z_OK)
                throw(std::runtime_error("inflateInit failed while decompressing."));

            zs.next_in = (Bytef*)str.data() + 1;
            zs.avail_in = str.size() - 1;

            int ret;
            char outbuffer[32768];
            std::string outstring;

            // get the decompressed bytes blockwise using repeated calls to inflate
            do {
                zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                zs.avail_out = sizeof(outbuffer);

                ret = inflate(&zs, 0);

                if (outstring.size() < zs.total_out) {
                    outstring.append(outbuffer,
                                     zs.total_out - outstring.size());
                }

            } while (ret == Z_OK);

            inflateEnd(&zs);

            if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
                std::ostringstream oss;
                oss << "Exception during zlib decompression: (" << ret << ") "
                    << zs.msg;
                throw(std::runtime_error(oss.str()));
            }

            return outstring;
        }
      };

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* JSON_VALUE_STORE_DEPRECATED_H_ */
