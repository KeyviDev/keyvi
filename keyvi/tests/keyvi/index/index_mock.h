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
 * index_mock.h
 *
 *  Created on: Jan 14, 2017
 *      Author: hendrik
 */

#ifndef KEYVI_TEST_INDEX_INDEX_MOCK_H_
#define KEYVI_TEST_INDEX_INDEX_MOCK_H_

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dictionary/compilation/compilation_utils.h"
#include "dictionary/fsa/automata.h"

namespace keyvi {
namespace testing {

class IndexMock final {
 public:
  IndexMock() : kv_files_(0) {
    using boost::filesystem::temp_directory_path;
    using boost::filesystem::unique_path;

    mock_index_ = temp_directory_path();
    mock_index_ /= unique_path();

    create_directories(mock_index_);
  }

  void AddSegment(std::vector<std::pair<std::string, std::string>>& input) {
    using boost::filesystem::path;

    path filename(mock_index_);
    filename /= "kv-";
    filename += std::to_string(kv_files_++);
    filename += ".kv";

    std::string filename_str = filename.string();

    dictionary::compilation::CompilationUtils::CompileJson(input, filename_str);

    WriteToc();
  }

  std::string GetIndexFolder() const { return mock_index_.string(); }

 private:
  boost::filesystem::path mock_index_;
  size_t kv_files_;

  void WriteToc() {
    boost::filesystem::path toc_file(mock_index_);
    toc_file /= "index.toc";

    std::ofstream toc(toc_file.string());
    std::stringstream ss;

    ss << "{\"files\": [";
    ss << "\"kv-0.kv\"";

    for (size_t i = 1; i < kv_files_; ++i) {
      ss << ", ";
      ss << "\"kv-";
      ss << std::to_string(i);
      ss << ".kv\"";
    }

    ss << "]}";

    toc << ss.str();
    toc.close();
  }
};

} /* namespace testing */
} /* namespace keyvi */

#endif /* KEYVI_TEST_INDEX_INDEX_MOCK_H_ */
