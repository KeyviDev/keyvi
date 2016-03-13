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

#ifndef TEMP_DICTIONARY_H_
#define TEMP_DICTIONARY_H_

#include <stdio.h>
#include <boost/filesystem.hpp>
#include <dictionary/compilation/compilation_utils.h>

namespace keyvi {
namespace dictionary {
namespace testing {

/***
 * Utility class for creating dictionaries for unit testing.
 * Ensures proper deletion after test completion.
 */
class TempDictionary
final {
   public:
    TempDictionary(std::vector<std::string>& input) {

      CreateFileName();
      fsa_ = compilation::CompilationUtils::CompileKeyOnly(input, file_name_);
    }

    TempDictionary(std::vector<std::pair<std::string, uint32_t>>& input) {

      CreateFileName();
      fsa_ = compilation::CompilationUtils::CompileIntWithInnerWeights(input, file_name_);
    }

    TempDictionary(std::vector<std::pair<std::string, std::string>>& input) {

      CreateFileName();
      fsa_ = compilation::CompilationUtils::CompileString(input, file_name_);
    }

    static TempDictionary makeTempDictionaryFromJson(std::vector<std::pair<std::string, std::string>>& input) {
      TempDictionary t;
      t.CreateFileName();
      t.fsa_ = compilation::CompilationUtils::CompileJson(input, t.file_name_);
      return t;
    }

    fsa::automata_t GetFsa() const {
      return fsa_;
    }

    const std::string& GetFileName() const {
      return file_name_;
    }

    ~TempDictionary() {
      std::remove(file_name_.c_str());
    }

   private:
    fsa::automata_t fsa_;
    std::string file_name_;

    TempDictionary(){}

    void CreateFileName() {
      boost::filesystem::path temp_path =
          boost::filesystem::temp_directory_path();

      temp_path /= boost::filesystem::unique_path(
          "dictionary-unit-test-temp-dictionary-%%%%-%%%%-%%%%-%%%%");
      file_name_ = temp_path.native();
    }

  };

  } /* namespace testing */
  } /* namespace dictionary */
  } /* namespace keyvi */

#endif /* TEMP_DICTIONARY_H_ */
