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
 * keyviinspector.cpp
 *
 *  Created on: May 13, 2014
 *      Author: hendrik
 */
#include <fstream>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>  // NOLINT(misc-include-cleaner)
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/entry_iterator.h"

void dump(const std::string& input, const std::string& output, bool keys_only = false) {
  keyvi::dictionary::fsa::automata_t const automata(new keyvi::dictionary::fsa::Automata(input));
  keyvi::dictionary::fsa::EntryIterator it(automata);
  keyvi::dictionary::fsa::EntryIterator const end_it = keyvi::dictionary::fsa::EntryIterator();

  std::ofstream out_stream(output);

  while (it != end_it) {
    it.WriteKey(out_stream);

    if (!keys_only) {
      std::string const value = it.GetValueAsString();
      if (!value.empty()) {
        out_stream << "\t";
        out_stream << value;
      }
    }
    out_stream << "\n";
    ++it;
  }
  out_stream.close();
}

void print_statistics(const std::string& input) {
  keyvi::dictionary::fsa::automata_t const automata(new keyvi::dictionary::fsa::Automata(input));
  std::cout << automata->GetStatistics() << '\n';
}

int main(int argc, char** argv) {
  std::string input_file;
  std::string output_file;

  boost::program_options::options_description description("keyvi inspector options:");

  description.add_options()("help,h", "Display this help message")("version,v", "Display the version number")(
      "input-file,i", boost::program_options::value<std::string>(), "input file")(
      "output-file,o", boost::program_options::value<std::string>(), "output file")(
      "keys-only,k", "dump only the keys")("statistics,s", "Show statistics of the file");

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

  bool key_only = false;
  if (vm.count("keys-only") != 0U) {
    key_only = true;
  }

  if ((vm.count("input-file") != 0U) && (vm.count("output-file") != 0U)) {
    input_file = vm["input-file"].as<std::string>();
    output_file = vm["output-file"].as<std::string>();

    dump(input_file, output_file, key_only);
    return 0;
  }

  if ((vm.count("input-file") != 0U) && (vm.count("statistics") != 0U)) {
    input_file = vm["input-file"].as<std::string>();
    print_statistics(input_file);
    return 0;
  }

  std::cout << "ERROR: arguments wrong or missing." << '\n' << '\n';
  std::cout << description;
  return 1;
}
