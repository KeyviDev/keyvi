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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/iterator_range.hpp>

#include "keyvi/dictionary/dictionary_compiler.h"
#include "keyvi/dictionary/dictionary_types.h"
#include "keyvi/util/configuration.h"

void callback(size_t added, size_t overall, void*) {
  std::cout << "Processed " << added << "/" << overall << "(" << ((100 * added) / overall) << "%)." << std::endl;
}

template <typename CompilerType, typename ValueType>
void compile_multiple(CompilerType* compiler, std::function<std::pair<std::string, ValueType>(std::string)> parser,
                      const std::vector<std::string>& inputs) {
  boost::iostreams::filtering_istream input_stream;
  std::string line;

  for (auto input_as_string : inputs) {
    auto input = boost::filesystem::path(input_as_string);

    if (boost::filesystem::is_directory(input)) {
      int files_added = 0;
      for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(input), {})) {
        if (entry.path().extension() == ".gz") {
          input_stream.push(boost::iostreams::gzip_decompressor());
        }

        boost::iostreams::file_source file(entry.path().string(), std::ios_base::in | std::ios_base::binary);
        input_stream.push(file);
        ++files_added;
        while (std::getline(input_stream, line)) {
          auto parse_result = parser(line);
          if (parse_result.first.size() == 0) {
            continue;
          }

          compiler->Add(parse_result.first, parse_result.second);
        }
        input_stream.reset();
      }

    } else {
      if (input.extension() == ".gz") {
        input_stream.push(boost::iostreams::gzip_decompressor());
      }

      boost::iostreams::file_source file(input.string(), std::ios_base::in | std::ios_base::binary);

      input_stream.push(file);
      while (std::getline(input_stream, line)) {
        auto parse_result = parser(line);
        compiler->Add(parse_result.first, parse_result.second);
      }
      input_stream.reset();
    }
  }
}

template <typename CompilerType>
void finalize_compile(CompilerType* compiler, const std::string& output, const std::string& manifest = {}) {
  std::ofstream out_stream(output, std::ios::binary);
  compiler->Compile(callback);
  compiler->SetManifest(manifest);

  compiler->Write(out_stream);
  out_stream.close();
}

template <class BucketT = uint32_t>
void compile_completion(const std::vector<std::string>& input, const std::string& output,
                        const std::string& manifest = {},
                        const keyvi::util::parameters_t& value_store_params = keyvi::util::parameters_t()) {
  keyvi::dictionary::CompletionDictionaryCompiler compiler(value_store_params);

  std::function<std::pair<std::string, uint32_t>(std::string)> parser = [](std::string line) {
    size_t tab = line.find('\t');

    if (tab == std::string::npos) return std::pair<std::string, uint32_t>();

    std::string key = line.substr(0, tab);
    std::string value_as_string = line.substr(tab + 1);
    uint32_t value;

    try {
      value = boost::lexical_cast<uint32_t>(value_as_string);
    } catch (boost::bad_lexical_cast const&) {
      std::cout << "Error: value was not valid: " << line << std::endl;
      return std::pair<std::string, uint32_t>();
    }
    return std::pair<std::string, uint32_t>(key, value);
  };
  compile_multiple(&compiler, parser, input);

  finalize_compile(&compiler, output, manifest);
}

void compile_integer(const std::vector<std::string>& input, const std::string& output, const std::string& manifest = {},
                     const keyvi::util::parameters_t& value_store_params = keyvi::util::parameters_t()) {
  keyvi::dictionary::IntDictionaryCompiler compiler(value_store_params);

  std::function<std::pair<std::string, uint32_t>(std::string)> parser = [](std::string line) {
    size_t tab = line.find('\t');

    if (tab == std::string::npos) return std::pair<std::string, uint32_t>();

    std::string key = line.substr(0, tab);
    std::string value_as_string = line.substr(tab + 1);
    uint32_t value;

    try {
      value = boost::lexical_cast<uint32_t>(value_as_string);
    } catch (boost::bad_lexical_cast const&) {
      std::cout << "Error: value was not valid: " << line << std::endl;
      return std::pair<std::string, uint32_t>();
    }
    return std::pair<std::string, uint32_t>(key, value);
  };
  compile_multiple(&compiler, parser, input);

  finalize_compile(&compiler, output, manifest);
}

template <class Compiler>
void compile_strings_inner(Compiler* compiler, const std::vector<std::string>& input, const std::string& output,
                           const std::string& manifest = {}) {
  std::function<std::pair<std::string, std::string>(std::string)> parser = [](std::string line) {
    size_t tab = line.find('\t');
    if (tab == std::string::npos) return std::pair<std::string, std::string>();
    std::string key = line.substr(0, tab);
    std::string value = line.substr(tab + 1);

    return std::pair<std::string, std::string>(key, value);
  };

  compile_multiple(compiler, parser, input);

  finalize_compile(compiler, output, manifest);
}

void compile_strings(const std::vector<std::string>& input, const std::string& output, const std::string& manifest = {},
                     const keyvi::util::parameters_t& value_store_params = keyvi::util::parameters_t()) {
  keyvi::dictionary::StringDictionaryCompiler compiler(value_store_params);
  compile_strings_inner(&compiler, input, output, manifest);
}

void compile_key_only(const std::vector<std::string>& input, const std::string& output,
                      const std::string& manifest = {},
                      const keyvi::util::parameters_t& value_store_params = keyvi::util::parameters_t()) {
  keyvi::dictionary::KeyOnlyDictionaryCompiler compiler(value_store_params);

  std::function<std::pair<std::string, uint32_t>(std::string)> parser = [](std::string line) {
    std::string key = line;
    size_t tab = line.find('\t');

    if (tab != std::string::npos) {
      key = line.substr(0, tab);
    }

    return std::pair<std::string, uint32_t>(key, 0);
  };

  compile_multiple(&compiler, parser, input);

  finalize_compile(&compiler, output, manifest);
}

void compile_json(const std::vector<std::string>& input, const std::string& output, const std::string& manifest = {},
                  const keyvi::util::parameters_t& value_store_params = keyvi::util::parameters_t()) {
  keyvi::dictionary::JsonDictionaryCompiler compiler(value_store_params);
  compile_strings_inner(&compiler, input, output, manifest);
}

/** Extracts the parameters. */
keyvi::util::parameters_t extract_parameters(const boost::program_options::variables_map& vm) {
  keyvi::util::parameters_t ret;
  for (auto& v : vm["parameter"].as<std::vector<std::string>>()) {
    std::vector<std::string> key_value;
    boost::split(key_value, v, std::bind(std::equal_to<char>(), std::placeholders::_1, '='));
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

  boost::program_options::options_description description("keyvi compiler options:");

  description.add_options()("help,h", "Display this help message")("version,v", "Display the version number");

  description.add_options()("input-file,i", boost::program_options::value<std::vector<std::string>>(), "input file");
  description.add_options()("output-file,o", boost::program_options::value<std::string>(), "output file");
  description.add_options()("memory-limit,m", boost::program_options::value<std::string>(),
                            "amount of main memory to use");
  description.add_options()("dictionary-type,d", boost::program_options::value<std::string>()->default_value("integer"),
                            "type of dictionary (integer (default), string, key-only, json, completion)");
  description.add_options()("parameter,p",
                            boost::program_options::value<std::vector<std::string>>()
                                ->default_value(std::vector<std::string>(), "EMPTY")
                                ->composing(),
                            "An option; format is -p xxx=yyy");

  description.add_options()("manifest", boost::program_options::value<std::string>()->default_value({}),
                            "manifest to be embedded");

  // Declare which options are positional
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  boost::program_options::variables_map vm;

  try {
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(description).run(),
                                  vm);

    boost::program_options::notify(vm);

    // parse positional options
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
      std::cout << description;
      return 0;
    }

    std::string manifest = vm["manifest"].as<std::string>();
    std::cout << manifest << std::endl;

    std::string dictionary_type = vm["dictionary-type"].as<std::string>();
    keyvi::util::parameters_t value_store_params = extract_parameters(vm);

    if (vm.count("memory-limit")) {
      value_store_params[MEMORY_LIMIT_KEY] = vm["memory-limit"].as<std::string>();
    }

    if (vm.count("input-file") && vm.count("output-file")) {
      input_files = vm["input-file"].as<std::vector<std::string>>();
      output_file = vm["output-file"].as<std::string>();

      if (dictionary_type == "integer") {
        compile_integer(input_files, output_file, manifest, value_store_params);
      } else if (dictionary_type == "string") {
        compile_strings(input_files, output_file, manifest, value_store_params);
      } else if (dictionary_type == "key-only") {
        compile_key_only(input_files, output_file, manifest, value_store_params);
      } else if (dictionary_type == "json") {
        compile_json(input_files, output_file, manifest, value_store_params);
      } else if (dictionary_type == "completion") {
        compile_completion(input_files, output_file, manifest, value_store_params);
      } else {
        std::cout << "ERROR: unknown dictionary type." << std::endl << std::endl;
        std::cout << description;
        return 1;
      }
    } else {
      std::cout << "ERROR: arguments wrong or missing." << std::endl << std::endl;
      std::cout << description;
      return 1;
    }
  } catch (std::exception& e) {
    std::cout << "ERROR: arguments wrong or missing." << std::endl << std::endl;

    std::cout << e.what() << std::endl << std::endl;
    std::cout << description;

    return 1;
  }
  return 0;
}
