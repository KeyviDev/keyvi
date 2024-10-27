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

#include <pybind11/pybind11.h>

#include <memory>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;

py::module_ msgpack_ = py::module_::import("msgpack");

void init_keyvi_match(const py::module_ &m) {
  py::class_<kd::Match, std::shared_ptr<kd::Match>>(m, "Match")
      .def(py::init<>())
      .def_property("start", &kd::Match::GetStart, &kd::Match::SetStart)
      .def_property("end", &kd::Match::GetEnd, &kd::Match::SetEnd)
      .def_property("score", &kd::Match::GetScore, &kd::Match::SetScore)
      .def_property("matched_string", &kd::Match::GetMatchedString, &kd::Match::SetMatchedString)
      .def_property_readonly("value",
                             [](const kd::Match &m) -> py::object {
                               auto packed_value = m.GetMsgPackedValueAsString();
                               if (packed_value.empty()) {
                                 return py::none();
                               }
                               return msgpack_.attr("loads")(packed_value);
                             })
      .def("value_as_string", &kd::Match::GetValueAsString)
      .def("raw_value_as_string", &kd::Match::GetRawValueAsString)
      .def("__get_item__", &kd::Match::GetAttributePy)
      // __setitem__
      // dumps loads
      .def_property_readonly("weight", &kd::Match::GetWeight)
      .def("__bool__", [](const kd::Match &m) -> bool { return !m.IsEmpty(); });
}
