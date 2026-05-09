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
#include <pybind11/stl.h>

#include <string>

#include "keyvi/dictionary/completion/forward_backward_completion.h"
#include "keyvi/dictionary/completion/multiword_completion.h"
#include "keyvi/dictionary/completion/prefix_completion.h"
#include "keyvi/dictionary/match.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;
namespace kdc = keyvi::dictionary::completion;
namespace kpy = keyvi::pybind;

void init_keyvi_completion(const py::module_& module) {
  py::class_<kdc::ForwardBackwardCompletion>(module, "ForwardBackwardCompletion")
      .def(py::init<kd::dictionary_t, kd::dictionary_t>(), py::arg("forward_dictionary"),
           py::arg("backward_dictionary"))
      .def(
          "complete",
          [](const kdc::ForwardBackwardCompletion& c, const std::string& query, const int number_of_results) {
            auto m = c.GetCompletions(query, number_of_results);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("number_of_results") = 10,
          R"pbdoc(
            Get forward-backward completions for a query.

            Returns an iterator of Match objects.
          )pbdoc");
  py::class_<kdc::MultiWordCompletion>(module, "MultiWordCompletion")
      .def(py::init<kd::dictionary_t>(), py::arg("dictionary"))
      .def(
          "complete",
          [](const kdc::MultiWordCompletion& c, const std::string& query, const int number_of_results) {
            auto m = c.GetCompletions(query, number_of_results);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("number_of_results") = 10,
          R"pbdoc(
            Get multi-word completions for a query.

            Returns an iterator of Match objects.
          )pbdoc");
  py::class_<kdc::PrefixCompletion>(module, "PrefixCompletion")
      .def(py::init<kd::dictionary_t>(), py::arg("dictionary"))
      .def(
          "complete",
          [](const kdc::PrefixCompletion& c, const std::string& query, const int number_of_results) {
            auto m = c.GetCompletions(query, number_of_results);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("number_of_results") = 10,
          R"pbdoc(
            Get prefix completions for a query.

            Returns an iterator of Match objects.
          )pbdoc")
      .def(
          "complete_fuzzy",
          [](const kdc::PrefixCompletion& c, const std::string& query, const int max_edit_distance,
             const size_t minimum_exact_prefix) {
            auto m = c.GetFuzzyCompletions(query, max_edit_distance, minimum_exact_prefix);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("max_edit_distance"), py::arg("minimum_exact_prefix") = 2,
          R"pbdoc(
            Get fuzzy prefix completions for a query.

            Returns an iterator of Match objects.
          )pbdoc");
}
