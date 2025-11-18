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

#include "keyvi/dictionary/completion/forward_backward_completion.h"
#include "keyvi/dictionary/completion/multiword_completion.h"
#include "keyvi/dictionary/completion/prefix_completion.h"
#include "keyvi/dictionary/match.h"

// #include "../py_match_iterator.h"

namespace py = pybind11;
namespace kdc = keyvi::dictionary::completion;
// namespace kpy = keyvi::pybind;

void init_keyvi_completion(const py::module_& module) {
  py::class_<kdc::ForwardBackwardCompletion>(module, "ForwardBackwardCompletion");
  py::class_<kdc::MultiWordCompletion>(module, "MultiWordCompletion");
  py::class_<kdc::PrefixCompletion>(module, "PrefixCompletion");
}
