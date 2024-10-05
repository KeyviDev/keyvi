/* * keyvi - A key value store.
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

#ifndef DICTIONARY_PY_MATCH_ITERATOR_H_
#define DICTIONARY_PY_MATCH_ITERATOR_H_

#include <pybind11/pybind11.h>

#include <utility>

namespace keyvi {
namespace pybind {

// adapted from pybind11.h
template <typename Access, pybind11::return_value_policy Policy, typename Iterator, typename Sentinel,
          typename ValueType, typename... Extra>
pybind11::iterator make_match_iterator_impl(Iterator first, Sentinel last, Extra &&...extra) {
  using state = pybind11::detail::iterator_state<Access, Policy, Iterator, Sentinel, ValueType, Extra...>;
  if (!pybind11::detail::get_type_info(typeid(state), false)) {
    pybind11::class_<state>(pybind11::handle(), "iterator", pybind11::module_local())
        .def("__iter__", [](state &s) -> state & { return s; })
        .def(
            "__next__",
            [](state &s) -> ValueType {
              {
                // release GIL as incrementing the iterator can be expensive, e.g. for fuzzy match
                pybind11::gil_scoped_release no_gil;
                if (!s.first_or_done) {
                  ++s.it;
                } else {
                  s.first_or_done = false;
                }
                if (s.it == s.end) {
                  s.first_or_done = true;
                  throw pybind11::stop_iteration();
                }
              }

              return Access()(s.it);
            },
            std::forward<Extra>(extra)..., Policy)
        .def("set_min_weight", [](state &s, const uint32_t min_weight) -> void { s.it.SetMinWeight(min_weight); });
  }

  return pybind11::cast(state{std::forward<Iterator>(first), std::forward<Sentinel>(last), true});
}

/// Makes a python iterator from a first and past-the-end C++ InputIterator.
template <pybind11::return_value_policy Policy = pybind11::return_value_policy::reference_internal, typename Iterator,
          typename Sentinel, typename ValueType = typename pybind11::detail::iterator_access<Iterator>::result_type,
          typename... Extra>
pybind11::typing::Iterator<ValueType> make_match_iterator(Iterator first, Sentinel last, Extra &&...extra) {
  return make_match_iterator_impl<pybind11::detail::iterator_access<Iterator>, Policy, Iterator, Sentinel, ValueType,
                                  Extra...>(std::forward<Iterator>(first), std::forward<Sentinel>(last),
                                            std::forward<Extra>(extra)...);
}

} /* namespace pybind */
} /* namespace keyvi */

#endif  // DICTIONARY_PY_MATCH_ITERATOR_H_
