from libcpp.string cimport string as libcpp_utf8_string
from dictionary cimport Dictionary
from std_smart_ptr cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "keyvi/dictionary/completion/forward_backward_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass ForwardBackwardCompletion:
        ForwardBackwardCompletion(shared_ptr[Dictionary], shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(libcpp_utf8_string)
        _MatchIteratorPair GetCompletions(libcpp_utf8_string, int)
