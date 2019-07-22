//
// keyvi - A key value store.
//
// Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/*
 * keyvi_parameters.h
 *
 *  Created on: Jan 19, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_UTIL_CONFIGURATION_H_
#define KEYVI_UTIL_CONFIGURATION_H_

#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "keyvi/dictionary/fsa/internal/constants.h"

namespace keyvi {
namespace util {

typedef std::map<std::string, std::string> parameters_t;

template <typename OutType>
OutType mapGet(const parameters_t& map, const std::string& key) {
  return boost::lexical_cast<OutType>(map.at(key));
}

template <typename OutType>
OutType mapGet(const parameters_t& map, const std::string& key, const OutType& default_value) {
  if (map.count(key) > 0) {
    return boost::lexical_cast<OutType>(map.at(key));
  }

  return default_value;
}

inline bool mapGetBool(const parameters_t& map, const std::string& key, const bool default_value) {
  if (map.count(key) > 0) {
    auto v = map.at(key);
    boost::algorithm::to_lower(v);
    if (v == "true" || v == "on") {
      return true;
    } else if (v == "false" || v == "off") {
      return false;
    }
  }

  return default_value;
}

inline size_t mapGetMemory(const parameters_t& map, const std::string& key, const size_t default_value) {
  if (map.count(key) > 0) {
    return boost::lexical_cast<size_t>(map.at(key));
  } else if (map.count(key + "_kb") > 0) {
    return 1024 * boost::lexical_cast<size_t>(map.at(key + "_kb"));
  } else if (map.count(key + "_mb") > 0) {
    return 1048576 * boost::lexical_cast<size_t>(map.at(key + "_mb"));
  } else if (map.count(key + "_gb") > 0) {
    return 1073741824 * boost::lexical_cast<size_t>(map.at(key + "_gb"));
  }

  return default_value;
}

inline std::string mapGetTemporaryPath(const parameters_t& map) {
  if (map.count(TEMPORARY_PATH_KEY) > 0) {
    return map.at(TEMPORARY_PATH_KEY);
  }

  return boost::filesystem::temp_directory_path().string();
}

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_CONFIGURATION_H_
