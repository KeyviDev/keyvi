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
 * serialization_utils.h
 *
 *  Created on: May 12, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_SERIALIZATION_UTILS_H_
#define KEYVI_UTIL_SERIALIZATION_UTILS_H_

#include <string>

#include <boost/lexical_cast.hpp>
// boost json parser depends on boost::spirit, and spirit is not thread-safe by default. so need to enable thread-safety
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "dictionary/util/endian.h"

namespace keyvi {
namespace util {

class SerializationUtils {
 public:
  static void WriteJsonRecord(std::ostream& stream, const boost::property_tree::ptree& properties) {
    std::stringstream string_buffer;

    boost::property_tree::write_json(string_buffer, properties, false);
    std::string header = string_buffer.str();

    uint32_t size = htobe32(header.size());

    stream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    stream << header;
  }

  static boost::property_tree::ptree ReadJsonRecord(std::istream& stream) {
    uint32_t header_size;
    stream.read(reinterpret_cast<char*>(&header_size), sizeof(int));
    header_size = be32toh(header_size);
    char* buffer = new char[header_size];
    stream.read(buffer, header_size);
    std::string buffer_as_string(buffer, header_size);
    delete[] buffer;
    std::istringstream string_stream(buffer_as_string);

    boost::property_tree::ptree properties;
    boost::property_tree::read_json(string_stream, properties);
    return properties;
  }

  static boost::property_tree::ptree ReadValueStoreProperties(std::istream& stream) {
    const auto properties = ReadJsonRecord(stream);
    const auto offset = stream.tellg();

    // check for file truncation
    const size_t vsSize = boost::lexical_cast<size_t>(properties.get<std::string>("size"));
    if (vsSize > 0) {
      stream.seekg(vsSize - 1, stream.cur);
      if (stream.peek() == EOF) {
        throw std::invalid_argument("file is corrupt(truncated)");
      }
    }

    stream.seekg(offset);
    return properties;
  }

  /**
   * Utility method to return a property tree from a JSON string.
   * @param record a string containing a JSON
   * @return the parsed property tree
   */
  static boost::property_tree::ptree ReadJsonRecord(const std::string& record) {
    boost::property_tree::ptree properties;

    // sending an empty string clears the manifest
    if (!record.empty()) {
      std::istringstream string_stream(record);
      boost::property_tree::read_json(string_stream, properties);
    }

    return properties;
  }
};

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_SERIALIZATION_UTILS_H_
