from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libc.stdint cimport int32_t
from dictionary cimport Dictionary
from std_smart_ptr cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/dictionary/completion/prefix_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass PrefixCompletion:
        PrefixCompletion(shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(libcpp_utf8_string)
        _MatchIteratorPair GetCompletions(libcpp_utf8_string, int)
        _MatchIteratorPair GetFuzzyCompletions(libcpp_utf8_string, int32_t max_edit_distance)
        _MatchIteratorPair GetFuzzyCompletions(libcpp_utf8_string, int32_t max_edit_distance, size_t minimum_exact_prefix)


