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
 * temp_dictionary.h
 *
 *  Created on: Jun 18, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_TESTING_TEMP_DICTIONARY_H_
#define KEYVI_TESTING_TEMP_DICTIONARY_H_

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

#include "keyvi/testing/compilation_utils.h"

namespace keyvi {
namespace testing {

/***
 * Utility class for creating dictionaries for unit testing.
 * Ensures proper deletion after test completion.
 */
class TempDictionary final {
 public:
  explicit TempDictionary(std::vector<std::string>* input) {
    CreateFileName();
    fsa_ = CompilationUtils::CompileKeyOnly(input, file_name_);
  }

  explicit TempDictionary(std::vector<std::pair<std::string, uint32_t>>* input, bool completion_dictionary = true) {
    CreateFileName();
    if (completion_dictionary) {
      fsa_ = CompilationUtils::CompileIntWithInnerWeights(input, file_name_);
    } else {
      fsa_ = CompilationUtils::CompileInt(input, file_name_);
    }
  }

  explicit TempDictionary(std::vector<std::pair<std::string, std::string>>* input) {
    CreateFileName();
    fsa_ = CompilationUtils::CompileString(input, file_name_);
  }

  static TempDictionary makeTempDictionaryFromJson(std::vector<std::pair<std::string, std::string>>* input) {
    TempDictionary t;
    t.CreateFileName();
    t.fsa_ = CompilationUtils::CompileJson(input, t.file_name_);
    return t;
  }

  static TempDictionary makeTempDictionaryFromFloats(std::vector<std::pair<std::string, std::vector<float>>>* input) {
    TempDictionary t;
    t.CreateFileName();
    t.fsa_ = CompilationUtils::CompileFloatVector(input, t.file_name_);
    return t;
  }

  dictionary::fsa::automata_t GetFsa() const { return fsa_; }

  const std::string& GetFileName() const { return file_name_; }

  ~TempDictionary() { std::remove(file_name_.c_str()); }

 private:
  dictionary::fsa::automata_t fsa_;
  std::string file_name_;

  TempDictionary() {}

  void CreateFileName() {
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

    temp_path /= boost::filesystem::unique_path("dictionary-unit-test-temp-dictionary-%%%%-%%%%-%%%%-%%%%");
    file_name_ = temp_path.string();
  }
};

} /* namespace testing */
} /* namespace keyvi */

#endif  // KEYVI_TESTING_TEMP_DICTIONARY_H_
