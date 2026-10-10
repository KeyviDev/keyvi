/* keyvi - A key value store.
 *
 * Copyright 2026 Hendrik Muhs<hendrik.muhs@gmail.com>
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

#include <map>
#include <string>

#include "keyvi/vector/vector_types.h"

namespace py = pybind11;
namespace kv = keyvi::vector;

using params_t = std::map<std::string, std::string>;

void init_keyvi_vector(const py::module_& m) {
  py::class_<kv::JsonVector>(m, "JsonVector")
      .def(py::init<const std::string&>())
      .def(
          "__getitem__",
          [](const kv::JsonVector& v, size_t index) {
            py::module_ json = py::module_::import("json");
            return json.attr("loads")(v.Get(index));
          },
          py::arg("index"))
      .def("__len__", &kv::JsonVector::Size)
      .def("manifest", &kv::JsonVector::Manifest);

  py::class_<kv::StringVector>(m, "StringVector")
      .def(py::init<const std::string&>())
      .def("__getitem__", &kv::StringVector::Get, py::arg("index"))
      .def("__len__", &kv::StringVector::Size)
      .def("manifest", &kv::StringVector::Manifest);

  py::class_<kv::JsonVectorGenerator>(m, "JsonVectorGenerator")
      .def(py::init<>())
      .def(py::init<const params_t&>())
      .def(
          "append",
          [](kv::JsonVectorGenerator& g, py::object value) {
            py::module_ json = py::module_::import("json");
            std::string dumped = json.attr("dumps")(value).cast<std::string>();
            g.PushBack(dumped);
          },
          py::arg("value"))
      .def("set_manifest", &kv::JsonVectorGenerator::SetManifest, py::arg("manifest"))
      .def("write_to_file", &kv::JsonVectorGenerator::WriteToFile, py::arg("filename"));

  py::class_<kv::StringVectorGenerator>(m, "StringVectorGenerator")
      .def(py::init<>())
      .def(py::init<const params_t&>())
      .def("append", &kv::StringVectorGenerator::PushBack, py::arg("value"))
      .def("set_manifest", &kv::StringVectorGenerator::SetManifest, py::arg("manifest"))
      .def("write_to_file", &kv::StringVectorGenerator::WriteToFile, py::arg("filename"));
}
