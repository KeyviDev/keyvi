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

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>

#include "keyvi/dictionary/dictionary_types.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;

template <typename Compiler>
inline void py_compile(Compiler *c, std::function<void(const size_t a, const size_t b)> progress_callback) {
  if (progress_callback == nullptr) {
    c->Compile();
    return;
  }
  auto progress_compiler_callback = [](size_t a, size_t b, void *user_data) {
    auto py_callback = *reinterpret_cast<std::function<void(const size_t, const size_t)> *>(user_data);
    py_callback(a, b);
  };
  void *user_data = reinterpret_cast<void *>(&progress_callback);
  c->Compile(progress_compiler_callback, user_data);
}

void init_keyvi_dictionary_compilers(const py::module_ &module) {
#define CREATE_COMPILER_COMMON(compiler)                                                                              \
  .def("__enter__", [](compiler &c) { return &c; })                                                                   \
      .def("__exit__", [](compiler &c, void *exc_type, void *exc_value, void *traceback) { c.Compile(); })            \
      .def("__setitem__", &compiler::Add)                                                                             \
      .def(                                                                                                           \
          "compile",                                                                                                  \
          [](compiler &c, std::function<void(const size_t a, const size_t b)> progress_callback) {                    \
            py_compile(&c, progress_callback);                                                                        \
          },                                                                                                          \
          py::arg("progress_callback") = static_cast<std::function<void(const size_t a, const size_t b)> *>(nullptr)) \
      .def(                                                                                                           \
          "Compile", /* DEPRECATED */                                                                                 \
          [](compiler &c, std::function<void(const size_t a, const size_t b)> progress_callback) {                    \
            py::module_ warnings = py::module_::import("warnings");                                                   \
            warnings.attr("warn")(                                                                                    \
                "Compile is deprecated and will be removed in a future version. Use compile instead.",                \
                py::module_::import("builtins").attr("DeprecationWarning"), 2);                                       \
            py_compile(&c, progress_callback);                                                                        \
          },                                                                                                          \
          py::arg("progress_callback") = static_cast<std::function<void(const size_t a, const size_t b)> *>(nullptr)) \
      .def("set_manifest", &compiler::SetManifest)                                                                    \
      .def("write_to_file", &compiler::WriteToFile, py::call_guard<py::gil_scoped_release>())                         \
      .def("WriteToFile", &compiler::WriteToFile, py::call_guard<py::gil_scoped_release>()) /* DEPRECATED */
#define CREATE_COMPILER(compiler, name)                                          \
  py::class_<compiler>(module, name)                                             \
      .def(py::init<>())                                                         \
      .def(py::init<const keyvi::util::parameters_t &>()) /* init with params */ \
      CREATE_COMPILER_COMMON(compiler)                                           \
      .def("add", &compiler::Add)                                                \
      .def("Add", &compiler::Add);
#define CREATE_KEY_ONLY_COMPILER(compiler, name)                                 \
  py::class_<compiler>(module, name)                                             \
      .def(py::init<>())                                                         \
      .def(py::init<const keyvi::util::parameters_t &>()) /* init with params */ \
      CREATE_COMPILER_COMMON(compiler)                                           \
      .def("add", [](compiler &c, const std::string &key) { c.Add(key); })       \
      .def("Add", [](compiler &c, const std::string &key) { c.Add(key); });
#define CREATE_SK_COMPILER(compiler, name)                                                  \
  py::class_<compiler>(module, name)                                                        \
      .def(py::init<const std::vector<std::string> &>())                                    \
      .def(py::init<const std::vector<std::string> &, const keyvi::util::parameters_t &>()) \
          CREATE_COMPILER_COMMON(compiler)                                                  \
      .def("add", &compiler::Add);
#define CREATE_MERGER(merger, name)                                                                    \
  py::class_<merger>(module, name)                                                                     \
      .def(py::init<>())                                                                               \
      .def(py::init<const keyvi::util::parameters_t &>())                                              \
      .def("__enter__", [](merger &m) { return &m; })                                                  \
      .def("__exit__", [](merger &m, void *exc_type, void *exc_value, void *traceback) { m.Merge(); }) \
      .def("add", &merger::Add)                                                                        \
      .def("merge",                                                                                    \
           [](merger &m) {                                                                             \
             pybind11::gil_scoped_release release_gil;                                                 \
             m.Merge();                                                                                \
           })                                                                                          \
      .def("merge",                                                                                    \
           [](merger &m, const std::string &filename) {                                                \
             pybind11::gil_scoped_release release_gil;                                                 \
             m.Merge(filename);                                                                        \
           })                                                                                          \
      .def("set_manifest", &merger::SetManifest)                                                       \
      .def("write_to_file", &merger::WriteToFile, py::call_guard<py::gil_scoped_release>());
  CREATE_COMPILER(kd::CompletionDictionaryCompiler, "CompletionDictionaryCompiler");
  CREATE_COMPILER(kd::FloatVectorDictionaryCompiler, "FloatVectorDictionaryCompiler");
  CREATE_COMPILER(kd::IntDictionaryCompiler, "IntDictionaryCompiler");
  CREATE_COMPILER(kd::JsonDictionaryCompiler, "JsonDictionaryCompiler");
  CREATE_KEY_ONLY_COMPILER(kd::KeyOnlyDictionaryCompiler, "KeyOnlyDictionaryCompiler");
  CREATE_COMPILER(kd::StringDictionaryCompiler, "StringDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyCompletionDictionaryCompiler, "SecondaryKeyCompletionDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyFloatVectorDictionaryCompiler, "SecondaryKeyFloatVectorDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyIntDictionaryCompiler, "SecondaryKeyIntDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyJsonDictionaryCompiler, "SecondaryKeyJsonDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyKeyOnlyDictionaryCompiler, "SecondaryKeyKeyOnlyDictionaryCompiler");
  CREATE_SK_COMPILER(kd::SecondaryKeyStringDictionaryCompiler, "SecondaryKeyStringDictionaryCompiler");
  CREATE_MERGER(kd::CompletionDictionaryMerger, "CompletionDictionaryMerger");
  CREATE_MERGER(kd::IntDictionaryMerger, "IntDictionaryMerger");

#undef CREATE_COMPILER
}
