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

#ifndef SERIALIZATION_UTILS_H_
#define SERIALIZATION_UTILS_H_

#include <arpa/inet.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace keyvi {
namespace dictionary {
namespace fsa {
namespace internal {

class SerializationUtils {
 public:
  static void WriteJsonRecord(std::ostream& stream,
                       boost::property_tree::ptree& properties) {
    std::stringstream string_buffer;

    boost::property_tree::write_json(string_buffer, properties, false);
    std::string header = string_buffer.str();

    uint32_t size = ntohl(header.size());

    stream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    stream << header;
  }

  static boost::property_tree::ptree ReadJsonRecord(std::istream& stream) {
    uint32_t header_size;
    stream.read(reinterpret_cast<char*>(&header_size), sizeof(int));
    header_size = htonl(header_size);
    char * buffer = new char[header_size];
    stream.read(buffer, header_size);
    std::string buffer_as_string(buffer, header_size);
    delete[] buffer;
    std::istringstream string_stream(buffer_as_string);

    boost::property_tree::ptree properties;
    boost::property_tree::read_json(string_stream, properties);
    return properties;
  }

  /**
   * Utility method to return a property tree from a JSON string.
   * @param record a string containing a JSON
   * @return the parsed property tree
   */
  static boost::property_tree::ptree ReadJsonRecord(const std::string& record){
    boost::property_tree::ptree properties;

    // sending an empty string clears the manifest
    if (!record.empty()) {
      std::istringstream string_stream(record);
      boost::property_tree::read_json(string_stream, properties);
    }

    return properties;
  }
};

} /* namespace internal */
} /* namespace fsa */
} /* namespace dictionary */
} /* namespace keyvi */

#endif /* SERIALIZATION_UTILS_H_ */
