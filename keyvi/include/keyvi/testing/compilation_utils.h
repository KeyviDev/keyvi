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
 * compilation_utils.h
 *
 *  Created on: Jun 18, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_TESTING_COMPILATION_UTILS_H_
#define KEYVI_TESTING_COMPILATION_UTILS_H_

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/dictionary/fsa/automata.h"
#include "keyvi/dictionary/fsa/generator.h"
#include "keyvi/dictionary/fsa/internal/float_vector_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_inner_weights_value_store.h"
#include "keyvi/dictionary/fsa/internal/int_value_store.h"
#include "keyvi/dictionary/fsa/internal/json_value_store.h"
#include "keyvi/dictionary/fsa/internal/sparse_array_persistence.h"
#include "keyvi/dictionary/fsa/internal/string_value_store.h"
#include "keyvi/util/configuration.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace testing {

class CompilationUtils {
 public:
  static dictionary::fsa::automata_t CompileKeyOnly(std::vector<std::string>* input, const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>> g(
        keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

    for (const auto& key : *input) {
      g.Add(key);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }

  static dictionary::fsa::automata_t CompileString(std::vector<std::pair<std::string, std::string>>* input,
                                                   const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>,
                               dictionary::fsa::internal::StringValueStore>
        g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

    for (const auto& pair : *input) {
      g.Add(pair.first, pair.second);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }

  static dictionary::fsa::automata_t CompileJson(std::vector<std::pair<std::string, std::string>>* input,
                                                 const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>,
                               dictionary::fsa::internal::JsonValueStore>
        g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

    for (const auto& pair : *input) {
      g.Add(pair.first, pair.second);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }

  static dictionary::fsa::automata_t CompileIntWithInnerWeights(std::vector<std::pair<std::string, uint32_t>>* input,
                                                                const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>,
                               dictionary::fsa::internal::IntInnerWeightsValueStore>
        g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

    for (const auto& pair : *input) {
      g.Add(pair.first, pair.second);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }

  static dictionary::fsa::automata_t CompileInt(std::vector<std::pair<std::string, uint32_t>>* input,
                                                const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>,
                               dictionary::fsa::internal::IntValueStore>
        g(keyvi::util::parameters_t({{"memory_limit_mb", "10"}}));

    for (const auto& pair : *input) {
      g.Add(pair.first, pair.second);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }

  static dictionary::fsa::automata_t CompileFloatVector(std::vector<std::pair<std::string, std::vector<float>>>* input,
                                                        const std::string& file_name) {
    std::sort(input->begin(), input->end());

    dictionary::fsa::internal::SparseArrayPersistence<> p(2048, boost::filesystem::temp_directory_path());

    dictionary::fsa::Generator<dictionary::fsa::internal::SparseArrayPersistence<>,
                               dictionary::fsa::internal::FloatVectorValueStore>
        g(keyvi::util::parameters_t(
            {{"memory_limit_mb", "10"}, {VECTOR_SIZE_KEY, std::to_string(input->at(0).second.size())}}));

    for (const auto& pair : *input) {
      g.Add(pair.first, pair.second);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    dictionary::fsa::automata_t f(new dictionary::fsa::Automata(file_name.c_str()));
    return f;
  }
};

} /* namespace testing */
} /* namespace keyvi */

#endif  // KEYVI_TESTING_COMPILATION_UTILS_H_
