from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp.vector cimport vector as libcpp_vector
from libcpp.pair  cimport pair  as libcpp_pair
from libcpp cimport bool
from match cimport Match
from libc.stdint cimport int32_t
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair
from libcpp.memory cimport shared_ptr

cdef extern from "keyvi/index/index.h" namespace "keyvi::index":
    cdef cppclass Index:
        Index(libcpp_utf8_string) except+ # wrap-ignore
        Index(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) except+ # wrap-ignore
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+ # wrap-as:set
        void MSet(shared_ptr[libcpp_vector[libcpp_pair[libcpp_utf8_string, libcpp_utf8_string]]]) except+ # wrap-ignore
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length) except + # wrap-as:get_near
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length, bool greedy) except + # wrap-as:get_near
        _MatchIteratorPair GetFuzzy(libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix) except + # wrap-as:get_fuzzy
        void Delete(libcpp_utf8_string) except+ # wrap-as:delete
        void Flush() except+ # wrap-as:flush
        void Flush(bool) except+ # wrap-as:flush
        bool Contains(libcpp_utf8_string) # wrap-ignore
        shared_ptr[Match] operator[](libcpp_utf8_string) # wrap-ignore
