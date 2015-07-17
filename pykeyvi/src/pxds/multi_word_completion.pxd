from libcpp.string cimport string
from libc.string cimport const_char
from dictionary cimport Dictionary
from  smart_ptr cimport shared_ptr
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair

cdef extern from "dictionary/completion/multiword_completion.h" namespace "keyvi::dictionary::completion":
    cdef cppclass MultiWordCompletion:
        MultiWordCompletion(shared_ptr[Dictionary]) except +
        _MatchIteratorPair GetCompletions(const_char*)
        _MatchIteratorPair GetCompletions(const_char*, int)


