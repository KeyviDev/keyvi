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
#include <pybind11/functional.h>

#include "keyvi/dictionary/dictionary_types.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;

void init_keyvi_dictionary_compilers(const py::module_ &m) {
  #define CREATE_COMPILER(compiler, name) \
        py::class_<compiler>(m, name) \
            .def(py::init<>()) \
            .def("__enter__", [](compiler &c) { return &c; }) \
            .def("__exit__", [](compiler &c, void *exc_type, void *exc_value, void *traceback) { c.Compile(); }) \
            .def("__setitem__", &compiler::Add) \
            .def("add", &compiler::Add) \
            .def("compile", [](compiler &c, std::function<void(const size_t a, const size_t b)> progress_callback) { \
              if (progress_callback == nullptr) { \
                  c.Compile(); \
                  return; \
              } \
              auto progress_compiler_callback = [](size_t a, size_t b, void *user_data) { \
                auto py_callback = *reinterpret_cast<std::function<void(const size_t, const size_t)>*>(user_data); \
                py_callback(a,b); \
              }; \
              void* user_data = reinterpret_cast<void*>(&progress_callback); \
              c.Compile(progress_compiler_callback, user_data); \
            }, py::arg("progress_callback") = static_cast<std::function<void(const size_t a, const size_t b)> *>(nullptr)) \
            .def("set_manifest", &compiler::SetManifest) \
            .def("write_to_file", &compiler::WriteToFile);

  CREATE_COMPILER(kd::CompletionDictionaryCompiler, "CompletionDictionaryCompiler");

  #undef CREATE_COMPILER
}

//cdef void progress_compiler_callback(size_t a, size_t b, void* py_callback) noexcept with gil:
//    (<object>py_callback)(a, b)