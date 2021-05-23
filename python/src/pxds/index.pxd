from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp.vector cimport vector as libcpp_vector
from libcpp.pair  cimport pair  as libcpp_pair
from libcpp cimport bool
from match cimport Match
from libc.stdint cimport int32_t
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair
from std_smart_ptr cimport shared_ptr as s_shared_ptr

cdef extern from "keyvi/index/index.h" namespace "keyvi::index":
    cdef cppclass Index:
        Index(libcpp_utf8_string) except+ # wrap-ignore
        Index(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) except+ # wrap-ignore
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+
        void MSet(s_shared_ptr[libcpp_vector[libcpp_pair[libcpp_utf8_string, libcpp_utf8_string]]]) # wrap-ignore
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length) except +
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length, bool greedy) except +
        _MatchIteratorPair GetFuzzy(libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix) except +
        void Delete(libcpp_utf8_string) except+
        void Flush() except+
        void Flush(bool) except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
