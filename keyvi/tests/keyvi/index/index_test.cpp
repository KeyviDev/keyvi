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
 * index_writer_test.cpp
 *
 *  Created on: Jan 13, 2017
 *      Author: hendrik
 */

#include <chrono>  //NOLINT
#include <cstdlib>
#include <thread>  //NOLINT

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "index/constants.h"
#include "index/index.h"
#include "testing/index_mock.h"

inline std::string get_keyvimerger_bin() {
  boost::filesystem::path path{std::getenv("KEYVI_UNITTEST_BASEPATH")};
  path /= "keyvimerger";

  BOOST_CHECK(boost::filesystem::is_regular_file(path));

  return path.string();
}

namespace keyvi {
namespace index {
BOOST_AUTO_TEST_SUITE(IndexTests)

// basic writer test, re-usable for testing different parameters
void basic_writer_test(const keyvi::util::parameters_t& params = keyvi::util::parameters_t()) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path();
  {
    Index writer(tmp_path.string(), params);

    writer.Set("a", "{\"id\":3}");

    writer.Set("b", "{\"id\":4}");
    writer.Flush();
    writer.Set("c", "{\"id\":5}");
    writer.Flush();
    writer.Set("d", "{\"id\":6}");
    writer.Flush();

    BOOST_CHECK(writer.Contains("a"));
    BOOST_CHECK(writer.Contains("b"));
    BOOST_CHECK(writer.Contains("c"));
    BOOST_CHECK(writer.Contains("d"));
  }
  boost::filesystem::remove_all(tmp_path);
}

BOOST_AUTO_TEST_CASE(basic_writer_default) {
  basic_writer_test({{KEYVIMERGER_BIN, get_keyvimerger_bin()}});
}

BOOST_AUTO_TEST_CASE(basic_writer_simple_merge_policy) {
  basic_writer_test({{KEYVIMERGER_BIN, get_keyvimerger_bin()}, {MERGE_POLICY, "simple"}});
}

void bigger_feed_test(const keyvi::util::parameters_t& params = keyvi::util::parameters_t()) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path("index-test-temp-index-%%%%-%%%%-%%%%-%%%%");
  {
    Index writer(tmp_path.string(), params);

    for (int i = 0; i < 10000; ++i) {
      writer.Set("a", "{\"id\":" + std::to_string(i) + "}");
      if (i % 50 == 0) {
        writer.FlushAsync();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    writer.Flush();
    BOOST_CHECK(writer.Contains("a"));
    dictionary::Match m = writer["a"];

    BOOST_CHECK_EQUAL("{\"id\":9999}", m.GetValueAsString());
  }
  boost::filesystem::remove_all(tmp_path);
}

BOOST_AUTO_TEST_CASE(bigger_feed) {
  bigger_feed_test(
      {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}, {"max_concurrent_merges", "2"}});
}

BOOST_AUTO_TEST_CASE(bigger_feed_simple_merge_policy) {
  bigger_feed_test({{"refresh_interval", "100"},
                    {KEYVIMERGER_BIN, get_keyvimerger_bin()},
                    {"max_concurrent_merges", "2"},
                    {KEYVIMERGER_BIN, get_keyvimerger_bin()},
                    {MERGE_POLICY, "simple"}});
}

BOOST_AUTO_TEST_CASE(index_reopen) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path("index-test-temp-index-%%%%-%%%%-%%%%-%%%%");
  {
    Index index(tmp_path.string(), {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});

    index.Set("a", "{\"id\":1}");
    index.Set("b", "{\"id\":2}");

    index.Flush();
    index.Set("c", "{\"id\":3}");

    BOOST_CHECK(index.Contains("a"));
  }

  {
    // reopen
    Index index(tmp_path.string(), {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});
    BOOST_CHECK(index.Contains("a"));
    BOOST_CHECK(index.Contains("b"));
    BOOST_CHECK(index.Contains("c"));

    index.Set("d", "{\"id\":4}");
    index.Delete("b");
  }

  {
    // reopen again
    Index index(tmp_path.string(), {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});
    BOOST_CHECK(index.Contains("a"));
    BOOST_CHECK(!index.Contains("b"));
    BOOST_CHECK(index.Contains("c"));
    BOOST_CHECK(index.Contains("d"));
  }

  boost::filesystem::remove_all(tmp_path);
}

BOOST_AUTO_TEST_CASE(index_reopen_deleted_keys) {
  testing::IndexMock mock_index;
  std::vector<std::pair<std::string, std::string>> test_data = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"abbcd", "{c:3}"}, {"abcde", "{a:1}"}, {"abdd", "{b:2}"},
  };
  mock_index.AddSegment(&test_data);

  std::vector<std::pair<std::string, std::string>> test_data_2 = {
      {"abbc", "{c:6}"}, {"babc", "{a:1}"}, {"babbc", "{b:2}"}, {"babcde", "{a:1}"}, {"babdd", "{b:2}"},
  };
  mock_index.AddSegment(&test_data_2);
  mock_index.AddDeletedKeys({"abbc", "abdd"}, 0);

  {
    Index index(mock_index.GetIndexFolder(), {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});
    BOOST_CHECK(index.Contains("abc"));
    BOOST_CHECK(!index.Contains("abdd"));
    BOOST_CHECK(index.Contains("abbc"));
    BOOST_CHECK(index.Contains("babc"));
    BOOST_CHECK(index.Contains("babcde"));
  }

  testing::IndexMock mock_index2;
  std::vector<std::pair<std::string, std::string>> test_data2 = {
      {"abc", "{a:1}"}, {"abbc", "{b:2}"}, {"abbcd", "{c:3}"}, {"abcde", "{a:1}"}, {"abdd", "{b:2}"},
  };
  mock_index2.AddSegment(&test_data2);

  std::vector<std::pair<std::string, std::string>> test_data2_2 = {
      {"abbc", "{c:6}"}, {"babc", "{a:1}"}, {"babbc", "{b:2}"}, {"babcde", "{a:1}"}, {"babdd", "{b:2}"},
  };
  mock_index2.AddSegment(&test_data2_2);
  mock_index2.AddDeletedKeys({"abbc", "abdd"}, 0);

  {
    Index index(mock_index.GetIndexFolder(), {{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});
    index.Delete("abc");
    index.Flush();
    BOOST_CHECK(!index.Contains("abc"));
    BOOST_CHECK(!index.Contains("abdd"));
    BOOST_CHECK(index.Contains("abbc"));
    BOOST_CHECK(index.Contains("babc"));
    BOOST_CHECK(index.Contains("babcde"));
  }
}

void index_with_deletes(const keyvi::util::parameters_t& params = keyvi::util::parameters_t()) {
  using boost::filesystem::temp_directory_path;
  using boost::filesystem::unique_path;

  auto tmp_path = temp_directory_path();
  tmp_path /= unique_path("index-test-temp-index-%%%%-%%%%-%%%%-%%%%");
  {
    Index index(tmp_path.string(), params);

    for (int i = 0; i < 100; ++i) {
      index.Set("a" + std::to_string(i), "{\"id\":" + std::to_string(i) + "}");
    }
    index.FlushAsync();

    // delete keys, also some non-existing ones
    for (int i = 20; i < 120; ++i) {
      index.Delete("a" + std::to_string(i));
    }

    for (int i = 0; i < 25; ++i) {
      index.Set("b" + std::to_string(i), "{\"id_b\":" + std::to_string(i) + "}");
    }
    index.Flush();

    for (int i = 25; i < 50; ++i) {
      index.Set("b" + std::to_string(i), "{\"id_b\":" + std::to_string(i) + "}");
    }

    for (int i = 20; i < 30; ++i) {
      index.Delete("b" + std::to_string(i));
    }

    index.Flush();
    for (int i = 45; i < 50; ++i) {
      index.Delete("b" + std::to_string(i));
    }
    index.Flush();

    for (int i = 0; i < 20; ++i) {
      BOOST_CHECK(index.Contains("a" + std::to_string(i)));
    }
    for (int i = 20; i < 80; ++i) {
      BOOST_CHECK(!index.Contains("a" + std::to_string(i)));
    }
    for (int i = 0; i < 20; ++i) {
      BOOST_CHECK(index.Contains("b" + std::to_string(i)));
    }
    for (int i = 20; i < 30; ++i) {
      BOOST_CHECK(!index.Contains("b" + std::to_string(i)));
    }
    for (int i = 30; i < 45; ++i) {
      BOOST_CHECK(index.Contains("b" + std::to_string(i)));
    }
    for (int i = 45; i < 50; ++i) {
      BOOST_CHECK(!index.Contains("a" + std::to_string(i)));
    }
  }

  boost::filesystem::remove_all(tmp_path);
}

BOOST_AUTO_TEST_CASE(index_delete_keys_defaults) {
  index_with_deletes({{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}});
}

BOOST_AUTO_TEST_CASE(index_delete_keys_simple_merge_policy) {
  index_with_deletes({{"refresh_interval", "100"}, {KEYVIMERGER_BIN, get_keyvimerger_bin()}, {MERGE_POLICY, "simple"}});
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace index
}  // namespace keyvi
