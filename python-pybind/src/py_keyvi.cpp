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

#include <pybind11/pybind11.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

void init_keyvi_dictionary(const py::module_ &);
void init_keyvi_match(const py::module_ &);

PYBIND11_MODULE(keyvi_scikit_core, m) {
  m.doc() = R"pbdoc(
        keyvi - a key value store.
        -----------------------

        .. currentmodule:: keyvi

        .. autosummary::
           :toctree: _generate

    )pbdoc";

  init_keyvi_match(m);
  py::module keyvi_dictionary = m.def_submodule("dictionary", "keyvi_scikit_core.dictionary");
  init_keyvi_dictionary(keyvi_dictionary);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
