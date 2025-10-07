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
 * keyvicompiler.cpp
 *
 *  Created on: May 13, 2014
 *      Author: hendrik
 */

#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/range/iterator_range_core.hpp>

#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/dictionary/fsa/internal/constants.h"
#include "keyvi/util/configuration.h"

/** Extracts the parameters. */
keyvi::util::parameters_t extract_parameters(const boost::program_options::variables_map& vm) {
  keyvi::util::parameters_t ret;
  for (const auto& v : vm["parameter"].as<std::vector<std::string>>()) {
    std::vector<std::string> key_value;
    boost::split(key_value, v, [](auto&& PH1) { return std::equal_to<char>()(std::forward<decltype(PH1)>(PH1), '='); });
    if (key_value.size() == 2) {
      ret[key_value[0]] = key_value[1];
    } else {
      throw std::invalid_argument("Invalid value store parameter format: " + v);
    }
  }
  return ret;
}

int main(int argc, char** argv) {
  std::vector<std::string> input_files;
  std::string output_file;

  boost::program_options::options_description description("keyvi merger options:");

  description.add_options()("help,h", "Display this help message")("version,v", "Display the version number");

  description.add_options()("input-file,i", boost::program_options::value<std::vector<std::string>>(), "input file");
  description.add_options()("output-file,o", boost::program_options::value<std::string>(), "output file");
  description.add_options()("memory-limit,m", boost::program_options::value<std::string>(),
                            "amount of main memory to use");
  description.add_options()("parameter,p",
                            boost::program_options::value<std::vector<std::string>>()
                                ->default_value(std::vector<std::string>(), "EMPTY")
                                ->composing(),
                            "An option; format is -p xxx=yyy");

  // Declare which options are positional
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(description).run(), vm);
  boost::program_options::notify(vm);

  // parse positional options
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
  boost::program_options::notify(vm);
  if (vm.count("help") != 0U) {
    std::cout << description;
    return 0;
  }

  if ((vm.count("input-file") != 0U) && (vm.count("output-file") != 0U)) {
    input_files = vm["input-file"].as<std::vector<std::string>>();
    output_file = vm["output-file"].as<std::string>();

    std::vector<std::string> inputs;

    for (const auto& f : input_files) {
      if (boost::filesystem::is_directory(f)) {
        for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(f), {})) {
          if (entry.path().extension() == ".kv") {
            inputs.push_back(entry.path().string());
          }
        }
      } else {
        inputs.push_back(f);
      }
    }

    keyvi::util::parameters_t params = extract_parameters(vm);
    if (vm.count("memory-limit") != 0U) {
      params[MEMORY_LIMIT_KEY] = vm["memory-limit"].as<std::string>();
    }

    keyvi::dictionary::JsonDictionaryMerger jsonDictionaryMerger(params);
    for (const auto& f : inputs) {
      jsonDictionaryMerger.Add(f);
    }

    jsonDictionaryMerger.Merge(output_file);

  } else {
    std::cout << "ERROR: arguments wrong or missing." << '\n' << '\n';
    std::cout << description;
    return 1;
  }

  return 0;
}
