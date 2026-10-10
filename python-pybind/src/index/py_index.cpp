/* keyvi - A key value store.
 *
 * Copyright 2024 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "keyvi/index/index.h"
#include "keyvi/index/read_only_index.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace ki = keyvi::index;
namespace kpy = keyvi::pybind;

using params_t = std::map<std::string, std::string>;

inline params_t inject_keyvimerger_bin(params_t params) {
  if (params.find("keyvimerger_bin") == params.end()) {
    py::object sys = py::module_::import("sys");
    std::string executable = sys.attr("executable").cast<std::string>();

    py::object os_path = py::module_::import("os.path");
    py::object keyvi2_module = py::module_::import("keyvi2");
    std::string module_file = keyvi2_module.attr("__file__").cast<std::string>();
    std::string module_dir = os_path.attr("dirname")(module_file).cast<std::string>();
    std::string merger_path = os_path.attr("join")(module_dir, "keyvimerger.py").cast<std::string>();

    params["keyvimerger_bin"] = executable + " " + merger_path;
  }
  return params;
}

void init_keyvi_index(const py::module_& module) {
  py::class_<ki::Index>(module, "Index")
      .def(py::init([](const std::string& index_directory) {
        return new ki::Index(index_directory, inject_keyvimerger_bin({}));
      }))
      .def(py::init([](const std::string& index_directory, const params_t& params) {
        return new ki::Index(index_directory, inject_keyvimerger_bin(params));
      }))
      .def("set", &ki::Index::Set, py::arg("key"), py::arg("value"))
      .def(
          "bulk_set",
          [](ki::Index& idx, const std::vector<std::pair<std::string, std::string>>& key_values) {
            auto kv = std::make_shared<std::vector<std::pair<std::string, std::string>>>(key_values.begin(),
                                                                                         key_values.end());
            idx.MSet(kv);
          },
          py::arg("key_values"))
      .def("delete", &ki::Index::Delete, py::arg("key"))
      .def("__delitem__", [](ki::Index& idx, const std::string& key) { idx.Delete(key); })
      .def("flush", &ki::Index::Flush, py::arg("async") = false)
      .def(
          "get",
          [](ki::Index& idx, const std::string& key, py::object default_value) -> py::object {
            auto m = idx[key];
            if (!m) {
              return default_value;
            }
            return py::cast(m);
          },
          py::arg("key"), py::arg("default") = py::none())
      .def("__getitem__",
           [](ki::Index& idx, const std::string& key) {
             auto m = idx[key];
             if (!m) {
               throw py::key_error(key);
             }
             return m;
           })
      .def("__contains__", [](ki::Index& idx, const std::string& key) { return idx.Contains(key); })
      .def(
          "get_near",
          [](ki::Index& idx, const std::string& key, const size_t minimum_prefix_length, const bool greedy) {
            auto m = idx.GetNear(key, minimum_prefix_length, greedy);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("minimum_prefix_length"), py::arg("greedy") = false)
      .def(
          "get_fuzzy",
          [](ki::Index& idx, const std::string& key, const int32_t max_edit_distance,
             const size_t minimum_exact_prefix) {
            auto m = idx.GetFuzzy(key, max_edit_distance, minimum_exact_prefix);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("max_edit_distance"), py::arg("minimum_exact_prefix") = 2);

  py::class_<ki::ReadOnlyIndex>(module, "ReadOnlyIndex")
      .def(py::init<const std::string&>())
      .def(py::init<const std::string&, const params_t&>())
      .def(
          "get",
          [](ki::ReadOnlyIndex& idx, const std::string& key, py::object default_value) -> py::object {
            auto m = idx[key];
            if (!m) {
              return default_value;
            }
            return py::cast(m);
          },
          py::arg("key"), py::arg("default") = py::none())
      .def("__getitem__",
           [](ki::ReadOnlyIndex& idx, const std::string& key) {
             auto m = idx[key];
             if (!m) {
               throw py::key_error(key);
             }
             return m;
           })
      .def("__contains__", [](ki::ReadOnlyIndex& idx, const std::string& key) { return idx.Contains(key); })
      .def(
          "get_near",
          [](ki::ReadOnlyIndex& idx, const std::string& key, const size_t minimum_prefix_length, const bool greedy) {
            auto m = idx.GetNear(key, minimum_prefix_length, greedy);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("minimum_prefix_length"), py::arg("greedy") = false)
      .def(
          "get_fuzzy",
          [](ki::ReadOnlyIndex& idx, const std::string& key, const int32_t max_edit_distance,
             const size_t minimum_exact_prefix) {
            auto m = idx.GetFuzzy(key, max_edit_distance, minimum_exact_prefix);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("max_edit_distance"), py::arg("minimum_exact_prefix") = 2);
}
