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
 * dictionarycompiler.cpp
 *
 *  Created on: May 13, 2014
 *      Author: hendrik
 */

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "dictionary/dictionary_compiler.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/string_value_store.h"
#include "dictionary/fsa/internal/json_value_store.h"

void callback (size_t added, size_t overall, void*) {
  std::cout << "Processed " << added << "/" << overall << "(" << ((100 * added) / overall) <<  "%)." << std::endl;
}

template<typename CompilerType, typename ValueType>
void compile_multiple(CompilerType& compiler, std::function<std::pair<std::string, ValueType>(std::string)> parser,
                      std::vector<std::string>& inputs)
{
  boost::iostreams::filtering_istream input_stream;
  std::string line;

  for (auto input_as_string : inputs) {
    auto input = boost::filesystem::path(input_as_string);

    if(boost::filesystem::is_directory(input)) {
      int files_added = 0;
      for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(input), {})) {
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

          compiler.Add(parse_result.first, parse_result.second);
        }
        input_stream.reset();
      }

    } else {
      if (input.extension() == ".gz"){
        input_stream.push(boost::iostreams::gzip_decompressor());
      }

      boost::iostreams::file_source file(input.string(), std::ios_base::in | std::ios_base::binary);

      input_stream.push(file);
      while (std::getline(input_stream, line)) {
        auto parse_result = parser(line);
        compiler.Add(parse_result.first, parse_result.second);
      }
      input_stream.reset();
    }
  }
}

template<typename CompilerType>
void finalize_compile(CompilerType& compiler, std::string& output,
                      size_t partition_size = 0) {
  if (partition_size == 0) {
    std::ofstream out_stream(output, std::ios::binary);
    compiler.Compile(callback);
    compiler.Write(out_stream);
    out_stream.close();
  } else {
    std::string output_part_zero = output + ".0";
    int partition_number = 1;
    std::ofstream out_stream(output_part_zero, std::ios::binary);

    while (compiler.CompileNext(partition_size, out_stream, 2, callback)) {
      std::cout << "Finalize partition " << partition_number << std::endl;

      out_stream.close();
      out_stream.open(output + "." + std::to_string(partition_number),
                      std::ios::binary);
      ++partition_number;
    }

    out_stream.close();
  }
}

template<class BucketT = uint32_t>
void compile_integer(std::vector<std::string>& input, std::string& output, size_t memory_limit,
             size_t partition_size = 0) {
  keyvi::dictionary::DictionaryCompiler<
      keyvi::dictionary::fsa::internal::SparseArrayPersistence<BucketT>,
      keyvi::dictionary::fsa::internal::IntValueStoreWithInnerWeights> compiler(
      memory_limit);

  std::function<std::pair<std::string, uint32_t>(std::string)> parser = [] (std::string line) {
    size_t tab = line.find('\t');

    if (tab == std::string::npos)
      return std::pair<std::string, uint32_t>();

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
  compile_multiple(compiler, parser, input);

  finalize_compile(compiler, output, partition_size);
}

template<class BucketT = uint32_t>
void compile_strings(std::vector<std::string>& input, std::string& output,
                     size_t memory_limit, size_t partition_size = 0) {
  keyvi::dictionary::DictionaryCompiler<
      keyvi::dictionary::fsa::internal::SparseArrayPersistence<BucketT>,
      keyvi::dictionary::fsa::internal::StringValueStore> compiler(
          memory_limit);

  std::function<std::pair<std::string, std::string>(std::string)> parser = [] (std::string line) {
    size_t tab = line.find('\t');
    if (tab == std::string::npos)
      return std::pair<std::string, std::string>();

    std::string key = line.substr(0, tab);
    std::string value = line.substr(tab + 1);

    return std::pair<std::string, std::string>(key, value);
  };

  compile_multiple(compiler, parser, input);

  finalize_compile(compiler, output, partition_size);
}

template<class BucketT = uint32_t>
void compile_key_only(std::vector<std::string>& input, std::string& output,
                      size_t memory_limit, size_t partition_size = 0) {
  keyvi::dictionary::DictionaryCompiler<
      keyvi::dictionary::fsa::internal::SparseArrayPersistence<BucketT>> compiler(
      memory_limit);

  std::function<std::pair<std::string, uint32_t>(std::string)> parser = [] (std::string line) {

    std::string key = line;
    size_t tab = line.find('\t');

    if (tab != std::string::npos) {
      key = line.substr(0, tab);
    }

    return std::pair<std::string, uint32_t>(key, 0);
  };

  compile_multiple(compiler, parser, input);

  finalize_compile(compiler, output, partition_size);
}

template<class BucketT = uint32_t>
void compile_json(std::vector<std::string>& input, std::string& output, size_t memory_limit,
                  size_t partition_size = 0) {
  keyvi::dictionary::DictionaryCompiler<
      keyvi::dictionary::fsa::internal::SparseArrayPersistence<BucketT>,
      keyvi::dictionary::fsa::internal::JsonValueStore> compiler(
          memory_limit);

  std::function<std::pair<std::string, std::string>(std::string)> parser = [] (std::string line) {
    size_t tab = line.find('\t');
    if (tab == std::string::npos)
      return std::pair<std::string, std::string>();

    std::string key = line.substr(0, tab);
    std::string value = line.substr(tab + 1);

    return std::pair<std::string, std::string>(key, value);
  };

  compile_multiple(compiler, parser, input);

  //input_stream.
  finalize_compile(compiler, output, partition_size);
}

int main(int argc, char** argv) {
  std::vector<std::string> input_files;
  std::string output_file;

  boost::program_options::options_description description(
      "dictionary compiler options:");

  description.add_options()("help,h", "Display this help message")(
      "version,v", "Display the version number");

  description.add_options()("input-file,i",
                            boost::program_options::value<std::vector<std::string>>(),
                            "input file");
  description.add_options()("output-file,o",
                            boost::program_options::value<std::string>(),
                            "output file");
  description.add_options()("memory-limit,m",
                            boost::program_options::value<size_t>(),
                            "amount of main memory to use");
  description.add_options()(
      "dictionary-type,d",
      boost::program_options::value<std::string>()->default_value("integer"),
      "type of dictionary (integer (default), string, key-only, json)");
  description.add_options()("partition-size,p",
                            boost::program_options::value<size_t>(),
                            "create partitions with a maximum size");
  description.add_options()("compact,c", "Compact Mode");

  // Declare which options are positional
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv).options(
          description).run(),
      vm);
  boost::program_options::notify(vm);

  // parse positional options
  boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv).options(
          description).positional(p).run(),
      vm);
  boost::program_options::notify(vm);
  if (vm.count("help")) {
    std::cout << description;
    return 0;
  }

  size_t memory_limit = 1073741824;
  if (vm.count("memory-limit")) {
    memory_limit = vm["memory-limit"].as<size_t>();
  }

  bool compact = false;
  if (vm.count("compact")) {
      compact = true;
  }

  size_t partition_size = 0;
  if (vm.count("partition-size")) {
    partition_size = vm["partition-size"].as<size_t>();
  }

  if (vm.count("input-file") && vm.count("output-file")) {
    input_files = vm["input-file"].as<std::vector<std::string>>();
    output_file = vm["output-file"].as<std::string>();

    std::string dictionary_type = vm["dictionary-type"].as<std::string>();
    if (dictionary_type == "integer") {
      if (compact){
        compile_integer<uint16_t>(input_files, output_file, memory_limit, partition_size);
      } else {
        compile_integer(input_files, output_file, memory_limit, partition_size);
      }
    } else if (dictionary_type == "string") {
      if (compact){
        compile_strings<uint16_t>(input_files, output_file, memory_limit, partition_size);
      } else {
        compile_strings(input_files, output_file, memory_limit, partition_size);
      }
    } else if (dictionary_type == "key-only") {
      if (compact){
        compile_key_only<uint16_t>(input_files, output_file, memory_limit, partition_size);
      } else {
        compile_key_only(input_files, output_file, memory_limit, partition_size);
      }
    } else if (dictionary_type == "json") {
      if (compact){
        compile_json<uint16_t>(input_files, output_file, memory_limit, partition_size);
      } else {
        compile_json(input_files, output_file, memory_limit, partition_size);
      }
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

  return 0;
}
