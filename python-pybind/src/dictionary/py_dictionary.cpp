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

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/secondary_key_dictionary.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;
namespace kpy = keyvi::pybind;

void init_keyvi_dictionary(const py::module_& m) {
  m.doc() = R"pbdoc(
        keyvi.dictionary
        -----------------------

        .. currentmodule:: keyvi.dictionary

        .. autosummary::
           :toctree: _generate

    )pbdoc";

  // TODO(hendrik): 'items', 'keys', 'manifest', 'match_fuzzy', 'match_near',
  // 'search_tokenized', 'statistics', 'values'
  py::class_<kd::Dictionary>(m, "Dictionary")
      .def(py::init<const std::string&>())
      .def(py::init<const std::string&, kd::loading_strategy_types>())
      .def(
          "complete_fuzzy_multiword",
          [](const kd::Dictionary& d, const std::string& query, const int32_t max_edit_distance,
             const size_t minimum_exact_prefix = 0, const unsigned char multiword_separator = 0x1b) {
            auto m = d.GetFuzzyMultiwordCompletion(query, max_edit_distance, minimum_exact_prefix, multiword_separator);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("max_edit_distance"), py::arg("minimum_exact_prefix") = 0,
          py::arg("multiword_separator") = 0x1b,
          R"pbdoc(Complete the given key to full matches after whitespace tokenizing,
                  allowing up to max_edit_distance distance(Levenshtein).
                  In case the used dictionary supports inner weights, the
                  completer traverses the dictionary according to weights,
                  otherwise byte-order.
          )pbdoc")
      .def(
          "complete_multiword",
          [](const kd::Dictionary& d, const std::string& query, const unsigned char multiword_separator = 0x1b) {
            auto m = d.GetMultiwordCompletion(query, multiword_separator);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("multiword_separator") = 0x1b,
          R"pbdoc(Complete the given key to full matches after whitespace tokenizing
                  and return the top n completions.
                  In case the used dictionary supports inner weights, the
                  completer traverses the dictionary according to weights,
                  otherwise byte-order.

                  Note, due to depth-first traversal the traverser
                  immediately yields results when it visits them. The results are
                  neither in order nor limited to n. It is up to the caller to resort
                  and truncate the lists of results.
                  Only the number of top completions is guaranteed.
          )pbdoc")
      .def(
          "complete_prefix",
          [](const kd::Dictionary& d, const std::string& query) {
            auto m = d.GetPrefixCompletion(query);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"),
          R"pbdoc(Complete the given key to full matches after whitespace tokenizing
                  and return the top n completions.
                  In case the used dictionary supports inner weights, the
                  completer traverses the dictionary according to weights,
                  otherwise byte-order.

                  Note, due to depth-first traversal the traverser
                  immediately yields results when it visits them. The results are
                  neither in order nor limited to n. It is up to the caller to resort
                  and truncate the lists of results.
                  Only the number of top completions is guaranteed.
          )pbdoc")
      .def(
          "complete_prefix",
          [](const kd::Dictionary& d, const std::string& query, size_t top_n) {
            auto m = d.GetPrefixCompletion(query, top_n);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("query"), py::arg("top_n"),
          R"pbdoc(Complete the given key to full matches after whitespace tokenizing
                  and return the top n completions.
                  In case the used dictionary supports inner weights, the
                  completer traverses the dictionary according to weights,
                  otherwise byte-order.

                  Note, due to depth-first traversal the traverser
                  immediately yields results when it visits them. The results are
                  neither in order nor limited to n. It is up to the caller to resort
                  and truncate the lists of results.
                  Only the number of top completions is guaranteed.
          )pbdoc")
      .def("get", &kd::Dictionary::operator[], R"pbdoc(
        Get an entry from the dictionary.
    )pbdoc")
      .def("__getitem__", &kd::Dictionary::operator[], R"pbdoc(
        Get an entry from the dictionary.
    )pbdoc")
      .def("match",
           [](const kd::Dictionary& d, const std::string& key) {
             auto m = d.Get(key);
             return kpy::make_match_iterator(m.begin(), m.end());
           })
      .def("search", &kd::Dictionary::Lookup);

  py::class_<kd::SecondaryKeyDictionary>(m, "SecondaryKeyDictionary");
}
