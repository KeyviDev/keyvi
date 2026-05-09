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

#include <string>

#include "keyvi/index/index.h"
#include "keyvi/index/read_only_index.h"

// #include "../py_match_iterator.h"

namespace py = pybind11;
namespace ki = keyvi::index;
// namespace kpy = keyvi::pybind;

void init_keyvi_index(const py::module_& module) {
  py::class_<ki::Index>(module, "Index");
  py::class_<ki::ReadOnlyIndex>(module, "ReadOnlyIndex");
}
