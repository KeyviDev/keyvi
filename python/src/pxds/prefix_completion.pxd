from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libc.stdint cimport int32_t
from dictionary cimport Dictionary
from libcpp.memory cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/dictionary/completion/prefix_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass PrefixCompletion:
        PrefixCompletion(shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(libcpp_utf8_string) # wrap-as:complete
        _MatchIteratorPair GetCompletions(libcpp_utf8_string, int) # wrap-as:complete
        _MatchIteratorPair GetFuzzyCompletions(libcpp_utf8_string, int32_t max_edit_distance) # wrap-as:complete_fuzzy
        _MatchIteratorPair GetFuzzyCompletions(libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix) # wrap-as:complete_fuzzy


