from libc.string cimport const_char
from libcpp.string cimport string as libcpp_string
from libc.stdint cimport uint32_t
from libcpp cimport bool
from match cimport Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "dictionary/dictionary.h" namespace "keyvi::dictionary":
    cdef cppclass Dictionary:
        Dictionary (const_char* filename) except +
        Dictionary (const_char* filename, bool load_lazy) except +
        bool Contains (const_char*) # wrap-ignore
        Match operator[](const_char*) # wrap-ignore
        _MatchIteratorPair Get (const_char*)
        _MatchIteratorPair GetNear (libcpp_string, size_t minimum_prefix_length) except +
        _MatchIteratorPair GetNear (libcpp_string, size_t minimum_prefix_length, bool greedy) except +
        _MatchIteratorPair GetAllItems () # wrap-ignore
        _MatchIteratorPair Lookup(const_char*)
        _MatchIteratorPair LookupText(const_char*)
        libcpp_string GetManifestAsString() except + # wrap-ignore
        libcpp_string GetStatistics() # wrap-ignore
        uint32_t GetSize() # wrap-ignore
