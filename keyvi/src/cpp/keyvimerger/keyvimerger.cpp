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

#include <boost/program_options.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/filesystem.hpp>
#include "dictionary/dictionary_types.h"

typedef keyvi::dictionary::fsa::internal::IValueStoreWriter::vs_param_t vs_param_t;

int main(int argc, char **argv) {
    std::vector<std::string> input_files;
    std::string output_file;

    boost::program_options::options_description description("keyvi merger options:");

    description.add_options()("help,h", "Display this help message")(
            "version,v", "Display the version number");

    description.add_options()("input-file,i",
                              boost::program_options::value<std::vector<std::string>>(),
                              "input file");
    description.add_options()("output-file,o",
                              boost::program_options::value<std::string>(),
                              "output file");

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

    if (vm.count("input-file") && vm.count("output-file")) {
        input_files = vm["input-file"].as<std::vector<std::string>>();
        output_file = vm["output-file"].as<std::string>();

        std::vector<std::string> inputs;

        for (auto f : input_files) {

            if (boost::filesystem::is_directory(f)) {
                int files_added = 0;
                for (auto &entry : boost::make_iterator_range(boost::filesystem::directory_iterator(f), {})) {
                    if (entry.path().extension() == ".kv") {
                        inputs.push_back(entry.path().string());
                    }
                }
            } else {
                inputs.push_back(f);
            }
        }

        typedef keyvi::dictionary::fsa::internal::IValueStoreWriter::vs_param_t vs_param_t;

        vs_param_t params;
        params["merge_mode"] = "append";

        keyvi::dictionary::JsonDictionaryMerger jsonDictionaryMerger(memory_limit, params);
        for (auto f : inputs) {
            jsonDictionaryMerger.Add(f);
        }

        jsonDictionaryMerger.Merge(output_file);

    } else {
        std::cout << "ERROR: arguments wrong or missing." << std::endl << std::endl;
        std::cout << description;
        return 1;
    }

    return 0;
}
