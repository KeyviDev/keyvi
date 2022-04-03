from libc.string cimport const_char
from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libcpp.string cimport string as libcpp_utf8_output_string
from libc.stdint cimport int32_t
from libc.stdint cimport uint64_t
from libcpp cimport bool
from match cimport Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/dictionary/dictionary.h" namespace "keyvi::dictionary":
    ctypedef enum loading_strategy_types:
        default_os, # no special treatment, use whatever the OS/Boost has as default
        lazy, # load data as needed with some read-ahead
        populate, # immediately load everything in memory (blocks until everything is fully read)
        populate_key_part, # populate only the key part, load value part lazy
        populate_lazy, # load data lazy but ask the OS to read ahead if possible (does not block)
        lazy_no_readahead, # disable any read-ahead (for cases when index > x * main memory)
        lazy_no_readahead_value_part, # disable read-ahead only for the value part
        populate_key_part_no_readahead_value_part # populate the key part, but disable read ahead value part
        
    cdef cppclass Dictionary:
        Dictionary (libcpp_utf8_string filename) except +
        Dictionary (libcpp_utf8_string filename, loading_strategy_types) except +
        bool Contains (libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
        _MatchIteratorPair Get (libcpp_utf8_string)
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length) except +
        _MatchIteratorPair GetNear (libcpp_utf8_string, size_t minimum_prefix_length, bool greedy) except +
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string, int32_t max_edit_distance) except +
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix) except +
        _MatchIteratorPair GetAllItems () # wrap-ignore
        _MatchIteratorPair Lookup(libcpp_utf8_string)
        _MatchIteratorPair LookupText(libcpp_utf8_string)
        libcpp_utf8_output_string GetManifest() except +
        libcpp_string GetStatistics() # wrap-ignore
        uint64_t GetSize() # wrap-ignore
