from libc.string cimport const_char
from libcpp.string cimport string as libcpp_string
from libcpp cimport bool
from match cimport Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "dictionary/dictionary.h" namespace "keyvi::dictionary":
    cdef cppclass Dictionary:
        Dictionary (const_char* filename) except +
        bool __contains__ (const_char*)
        Match operator[](const_char*) # wrap-ignore
        _MatchIteratorPair Get (const_char*)
        _MatchIteratorPair GetAllItems () # wrap-ignore
        _MatchIteratorPair Lookup(const_char*)
        _MatchIteratorPair LookupText(const_char*)
        libcpp_string GetManifestAsString()
