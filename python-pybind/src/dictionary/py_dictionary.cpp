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

#include <cstdint>
#include <memory>
#include <string>

#include "keyvi/dictionary/dictionary.h"
#include "keyvi/dictionary/match.h"
#include "keyvi/dictionary/secondary_key_dictionary.h"

#include "py_match_iterator.h"

namespace py = pybind11;
namespace kd = keyvi::dictionary;
namespace kpy = keyvi::pybind;

inline const py::object& get_msgpack_loads_func() {
  PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
  return storage
      .call_once_and_store_result([]() -> py::object { return py::getattr(py::module_::import("msgpack"), "loads"); })
      .get_stored();
}

inline py::object match_value(const kd::Match& m) {
  auto packed_value = m.GetMsgPackedValueAsString();
  if (packed_value.empty()) {
    return py::none();
  }
  return get_msgpack_loads_func()(py::bytes(packed_value));
}

struct DictionaryItemsIterator {
  kd::MatchIterator it;
  kd::MatchIterator end;
};

void init_keyvi_dictionary(const py::module_& m) {
  m.doc() = R"pbdoc(
        keyvi.dictionary
        -----------------------

        .. currentmodule:: keyvi.dictionary

        .. autosummary::
           :toctree: _generate

    )pbdoc";

  py::class_<DictionaryItemsIterator>(m, "DictionaryItemsIterator")
      .def("__iter__", [](DictionaryItemsIterator& s) -> DictionaryItemsIterator& { return s; })
      .def("__next__", [](DictionaryItemsIterator& s) -> py::tuple {
        if (s.it == s.end) {
          throw py::stop_iteration();
        }
        const kd::match_t& m = *s.it;
        py::tuple result = py::make_tuple(m->GetMatchedString(), match_value(*m));
        ++s.it;
        return result;
      });

  py::class_<kd::Dictionary, std::shared_ptr<kd::Dictionary>>(m, "Dictionary")
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
      .def(
          "__getitem__",
          [](const kd::Dictionary& d, const std::string& key) {
            auto m = d[key];
            if (!m) {
              throw py::key_error(key);
            }
            return m;
          },
          R"pbdoc(
        Get an entry from the dictionary. Raises KeyError if not found.
    )pbdoc")
      .def(
          "__contains__", [](const kd::Dictionary& d, const std::string& key) { return d.Contains(key); },
          R"pbdoc(
        Check if a key is in the dictionary.
    )pbdoc")
      .def("__len__", &kd::Dictionary::GetSize, R"pbdoc(
        Return the number of keys in the dictionary.
    )pbdoc")
      .def(
          "items",
          [](const kd::Dictionary& d) {
            auto m = d.GetAllItems();
            return DictionaryItemsIterator{m.begin(), m.end()};
          },
          R"pbdoc(
            Return an iterator over all (key, value) tuples in the dictionary.
          )pbdoc")
      .def(
          "match",
          [](const kd::Dictionary& d, const std::string& key) {
            auto m = d.Get(key);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), R"pbdoc(
            Exact match for a key.
          )pbdoc")
      .def(
          "match_fuzzy",
          [](const kd::Dictionary& d, const std::string& key, const int32_t max_edit_distance,
             const size_t minimum_exact_prefix) {
            auto m = d.GetFuzzy(key, max_edit_distance, minimum_exact_prefix);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("max_edit_distance"), py::arg("minimum_exact_prefix") = 2,
          R"pbdoc(
            Fuzzy match for a key allowing up to max_edit_distance Levenshtein distance.
          )pbdoc")
      .def(
          "match_near",
          [](const kd::Dictionary& d, const std::string& key, const size_t minimum_prefix_length, const bool greedy) {
            auto m = d.GetNear(key, minimum_prefix_length, greedy);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("key"), py::arg("minimum_prefix_length"), py::arg("greedy") = false,
          R"pbdoc(
            Match a key near: match as much as possible exact given the minimum
            prefix length and then return everything below.

            If greedy is True it matches everything below the minimum_prefix_length,
            but in the order of exact first.
          )pbdoc")
      .def("manifest", &kd::Dictionary::GetManifest, R"pbdoc(
        Get the manifest of the dictionary.
    )pbdoc")
      .def("search", &kd::Dictionary::Lookup, py::arg("key"), py::arg("offset") = 0, R"pbdoc(
        Search for a key using leftmost longest lookup.
    )pbdoc")
      .def(
          "search_tokenized",
          [](kd::Dictionary& d, const std::string& text) {
            auto m = d.LookupText(text);
            return kpy::make_match_iterator(m.begin(), m.end());
          },
          py::arg("text"),
          R"pbdoc(
            Search a text by tokenizing on whitespace and performing
            leftmost longest lookup for each token.
          )pbdoc")
      .def(
          "statistics",
          [](const kd::Dictionary& d) {
            py::module_ json = py::module_::import("json");
            return json.attr("loads")(d.GetStatistics());
          },
          R"pbdoc(
            Get the statistics of the dictionary as a python dict.
          )pbdoc");

  py::class_<kd::SecondaryKeyDictionary>(m, "SecondaryKeyDictionary");
}
