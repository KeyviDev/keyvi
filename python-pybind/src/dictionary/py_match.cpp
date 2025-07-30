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

#include "msgpack.hpp"

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;

inline const py::object &get_msgpack_loads_func() {
  PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
  return storage
      .call_once_and_store_result([]() -> py::object { return py::getattr(py::module_::import("msgpack"), "loads"); })
      .get_stored();
}

void init_keyvi_match(const py::module_ &m) {
  py::module_ msgpack_ = py::module_::import("msgpack");

  py::class_<kd::Match, std::shared_ptr<kd::Match>>(m, "Match")
      .def(py::init<>())
      .def_property("start", &kd::Match::GetStart, &kd::Match::SetStart)
      .def_property("end", &kd::Match::GetEnd, &kd::Match::SetEnd)
      .def_property("score", &kd::Match::GetScore, &kd::Match::SetScore)
      .def_property("matched_string", &kd::Match::GetMatchedString, &kd::Match::SetMatchedString)
      .def_property_readonly("value",
                             [&msgpack_](const kd::Match &m) -> py::object {
                               auto packed_value = m.GetMsgPackedValueAsString();
                               if (packed_value.empty()) {
                                 return py::none();
                               }
                               return get_msgpack_loads_func()(py::bytes(packed_value));
                             })
      .def("value_as_string", &kd::Match::GetValueAsString)
      .def("raw_value_as_string", &kd::Match::GetRawValueAsString)
      .def("__getitem__", [](kd::Match &m, const std::string &key) {
        return m.GetAttribute(key);
      })
      .def("__setitem__", &kd::Match::SetAttribute<std::string>)
      .def("__setitem__", &kd::Match::SetAttribute<float>)
      .def("__setitem__", &kd::Match::SetAttribute<int>)
      .def("__setitem__", &kd::Match::SetAttribute<bool>)
      .def("dumps",
           [](const kd::Match &m) -> py::bytes {
             bool do_pack_rest = false;
             msgpack::sbuffer msgpack_buffer;
             msgpack::packer<msgpack::sbuffer> packer(&msgpack_buffer);
             const double score = m.GetScore();
             const size_t end = m.GetEnd();
             const size_t start = m.GetStart();
             const std::string matched_string = m.GetMatchedString();
             const std::string raw_value = m.GetRawValueAsString();

             const size_t array_size = score > 0                   ? 5
                                       : end > 0                   ? 4
                                       : start > 0                 ? 3
                                       : matched_string.size() > 0 ? 2
                                       : raw_value.size() > 0      ? 1
                                                                   : 0;
             packer.pack_array(array_size);

             if (array_size > 0) {
               packer.pack(raw_value);
             }
             if (array_size > 1) {
               packer.pack(matched_string);
             }
             if (array_size > 2) {
               packer.pack(start);
             }
             if (array_size > 3) {
               packer.pack(end);
             }
             if (array_size > 4) {
               packer.pack(score);
             }

             return py::bytes(msgpack_buffer.data(), msgpack_buffer.size());
           })
      .def_static("loads",
                  [](const std::string_view &serialized_match) -> kd::Match {
                    kd::Match match;
                    msgpack::object_handle handle = msgpack::unpack(serialized_match.data(), serialized_match.size());
                    msgpack::object obj = handle.get();

                    // Ensure it's an array
                    if (obj.type != msgpack::type::ARRAY) {
                      throw std::invalid_argument("not a serialized match");
                    }

                    // Get the array elements
                    const msgpack::object *array = obj.via.array.ptr;
                    uint32_t size = obj.via.array.size;

                    if (size > 5) {
                      throw std::invalid_argument("not a serialized match, unexpected number of elements");
                    }

                    std::string matched_string, value;
                    double score;
                    size_t start, end;

                    try {
                      switch (size) {
                        case 5:
                          array[4].convert(score);
                          match.SetScore(score);
                        case 4:
                          array[3].convert(end);
                          match.SetEnd(end);
                        case 3:
                          array[2].convert(start);
                          match.SetStart(start);
                        case 2:
                          array[1].convert(matched_string);
                          match.SetMatchedString(matched_string);
                        case 1:
                          array[0].convert(value);
                          match.SetRawValue(value);
                      }
                    } catch (const msgpack::type_error &e) {
                      throw std::invalid_argument("not a serialized match, unexpected element types");
                    }
                    return match;
                  })
      .def_property_readonly("weight", &kd::Match::GetWeight)
      .def("__bool__", [](const kd::Match &m) -> bool { return !m.IsEmpty(); });
}
