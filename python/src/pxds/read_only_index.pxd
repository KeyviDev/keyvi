from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp cimport bool
from libc.stdint cimport int32_t
from match cimport Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/index/read_only_index.h" namespace "keyvi::index":
    cdef cppclass ReadOnlyIndex:
        ReadOnlyIndex(libcpp_utf8_string) except+
        ReadOnlyIndex(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
        _MatchIteratorPair GetFuzzy(libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix) except+
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length) except +
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length, bool greedy) except +
