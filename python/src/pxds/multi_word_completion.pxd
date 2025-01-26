from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from dictionary cimport Dictionary
from libcpp.memory cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/dictionary/completion/multiword_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass MultiWordCompletion:
        MultiWordCompletion(shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(libcpp_utf8_string) # wrap-as:complete
        _MatchIteratorPair GetCompletions(libcpp_utf8_string, int) # wrap-as:complete


