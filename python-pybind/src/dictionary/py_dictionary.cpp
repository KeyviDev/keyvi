/* * keyvi - A key value store.
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

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"

#include "py_match_iterator.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;
namespace kd = keyvi::dictionary;
namespace kpy = keyvi::pybind;

void init_keyvi_dictionary(const py::module_ &m) {
  m.doc() = R"pbdoc(
        keyvi.dictionary
        -----------------------

        .. currentmodule:: keyvi.dictionary

        .. autosummary::
           :toctree: _generate

    )pbdoc";

  py::class_<kd::Dictionary>(m, "Dictionary")
      .def(py::init<const std::string &>())
      .def("get", &kd::Dictionary::operator[], R"pbdoc(
        Get an entry from the dictionary.
    )pbdoc")
      .def("search", &kd::Dictionary::Lookup)
      .def("match", [](const kd::Dictionary &d, const std::string &key) {
        auto m = d.Get(key);
        return kpy::make_match_iterator(m.begin(), m.end());
      });
}
