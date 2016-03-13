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

#ifndef COMPILATION_UTILS_H_
#define COMPILATION_UTILS_H_

#include "dictionary/fsa/automata.h"
#include "dictionary/fsa/internal/sparse_array_persistence.h"
#include "dictionary/fsa/generator.h"
#include "dictionary/fsa/internal/int_value_store.h"
#include "dictionary/fsa/internal/string_value_store.h"
#include "dictionary/fsa/internal/json_value_store.h"

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace compilation {

class CompilationUtils {
 public:
  static fsa::automata_t CompileKeyOnly(std::vector<std::string>& input, std::string& file_name){
    std::sort(input.begin(), input.end());

    fsa::internal::SparseArrayPersistence<> p(2048,
                                         boost::filesystem::temp_directory_path());

    fsa::Generator<fsa::internal::SparseArrayPersistence<>> g;

    for(auto key : input){
      g.Add(key);
    }

    g.CloseFeeding();
    std::ofstream out_stream(file_name, std::ios::binary);
    g.Write(out_stream);
    out_stream.close();

    fsa::automata_t f(new fsa::Automata(file_name.c_str()));
    return f;
  }

  static fsa::automata_t CompileString(std::vector<std::pair<std::string, std::string>>& input, std::string& file_name){
      std::sort(input.begin(), input.end());

      fsa::internal::SparseArrayPersistence<> p(2048,
                                           boost::filesystem::temp_directory_path());

      fsa::Generator<fsa::internal::SparseArrayPersistence<>, fsa::internal::StringValueStore> g;

      for(auto pair : input){
        g.Add(pair.first, pair.second);
      }

      g.CloseFeeding();
      std::ofstream out_stream(file_name, std::ios::binary);
      g.Write(out_stream);
      out_stream.close();

      fsa::automata_t f(new fsa::Automata(file_name.c_str()));
      return f;
    }

  static fsa::automata_t CompileJson(std::vector<std::pair<std::string, std::string>>& input, std::string& file_name){
        std::sort(input.begin(), input.end());

        fsa::internal::SparseArrayPersistence<> p(2048,
                                             boost::filesystem::temp_directory_path());

        fsa::Generator<fsa::internal::SparseArrayPersistence<>, fsa::internal::JsonValueStore> g;

        for(auto pair : input){
          g.Add(pair.first, pair.second);
        }

        g.CloseFeeding();
        std::ofstream out_stream(file_name, std::ios::binary);
        g.Write(out_stream);
        out_stream.close();

        fsa::automata_t f(new fsa::Automata(file_name.c_str()));
        return f;
      }

  static fsa::automata_t CompileIntWithInnerWeights(std::vector<std::pair<std::string, uint32_t>>& input, std::string& file_name){
      std::sort(input.begin(), input.end());

      fsa::internal::SparseArrayPersistence<> p(2048,
                                           boost::filesystem::temp_directory_path());

      fsa::Generator<fsa::internal::SparseArrayPersistence<>, fsa::internal::IntValueStoreWithInnerWeights> g;

      for(auto pair : input){
        g.Add(pair.first, pair.second);
      }

      g.CloseFeeding();
      std::ofstream out_stream(file_name, std::ios::binary);
      g.Write(out_stream);
      out_stream.close();

      fsa::automata_t f(new fsa::Automata(file_name.c_str()));
      return f;
    }

};

} /* namespace compilation */
} /* namespace dictionary */
} /* namespace keyvi */


#endif /* COMPILATION_UTILS_H_ */
