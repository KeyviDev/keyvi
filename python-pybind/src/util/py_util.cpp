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

#include <cstdint>
#include <memory>
#include <string>

#include "keyvi/compression/predictive_compression.h"
#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/util/jump_consistent_hash.h"
#include "keyvi/transform/fsa_transform.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;
namespace kt = keyvi::transform;
namespace kc = keyvi::compression;

void init_keyvi_util(py::module_& m) {
  m.def("JumpConsistentHashString", &kd::util::JumpConsistentHashString, py::arg("key"), py::arg("num_buckets"));

  py::class_<kt::FsaTransform>(m, "FsaTransform")
      .def(py::init<std::shared_ptr<kd::Dictionary>>())
      .def("normalize", &kt::FsaTransform::Normalize, py::arg("input"));

  py::class_<kc::PredictiveCompression>(m, "PredictiveCompression")
      .def(py::init<std::string>())
      .def("compress", &kc::PredictiveCompression::Compress, py::arg("input"))
      .def("uncompress", &kc::PredictiveCompression::Uncompress, py::arg("input"));
}
