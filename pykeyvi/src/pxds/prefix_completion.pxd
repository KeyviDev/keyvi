from libcpp.string cimport string as libcpp_string
from dictionary cimport Dictionary
from  smart_ptr cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "dictionary/completion/prefix_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass PrefixCompletion:
        PrefixCompletion(shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(libcpp_string)
        _MatchIteratorPair GetFuzzyCompletions(libcpp_string, int max_edit_distance)


