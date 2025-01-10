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

#include "keyvi/dictionary/fsa/internal/memory_map_flags.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace kd = keyvi::dictionary;

void init_keyvi_dictionary(const py::module_ &);
void init_keyvi_dictionary_compilers(const py::module_ &);
void init_keyvi_match(const py::module_ &);

PYBIND11_MODULE(keyvi_scikit_core, m) {
  m.doc() = R"pbdoc(
        keyvi - a key value store.
        -----------------------

        .. currentmodule:: keyvi

        .. autosummary::
           :toctree: _generate

    )pbdoc";

  py::enum_<kd::loading_strategy_types>(m, "loading_strategy_types")
      .value("default_os", kd::loading_strategy_types::default_os)
      .value("lazy", kd::loading_strategy_types::lazy)
      .value("populate", kd::loading_strategy_types::populate)
      .value("populate_key_part", kd::loading_strategy_types::populate_key_part)
      .value("populate_lazy", kd::loading_strategy_types::populate_lazy)
      .value("lazy_no_readahead", kd::loading_strategy_types::lazy_no_readahead)
      .value("lazy_no_readahead_value_part", kd::loading_strategy_types::lazy_no_readahead_value_part)
      .value("populate_key_part_no_readahead_value_part",
             kd::loading_strategy_types::populate_key_part_no_readahead_value_part);

  init_keyvi_match(m);
  py::module keyvi_dictionary = m.def_submodule("dictionary", "keyvi_scikit_core.dictionary");
  init_keyvi_dictionary(keyvi_dictionary);
  py::module keyvi_compilers = m.def_submodule("compiler", "keyvi_scikit_core.compiler");
  init_keyvi_dictionary_compilers(keyvi_compilers);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
