# same import style as autowrap
from match cimport Match as _Match
from libc.stdint cimport uint32_t
from libcpp.memory cimport shared_ptr

cdef extern from "keyvi/dictionary/match_iterator.h" namespace "keyvi::dictionary":
    cdef cppclass MatchIterator:
        # wrap-ignore
        shared_ptr[_Match] operator*()
        # wrap-ignore
        MatchIterator& operator++()
        bint operator==(MatchIterator)
        bint operator!=(MatchIterator)
        void SetMinWeight(uint32_t) # wrap-ignore

cdef extern from "keyvi/dictionary/match_iterator.h" namespace "keyvi::dictionary::MatchIterator":
    cdef cppclass MatchIteratorPair:
        # wrap-ignore
        MatchIterator begin()
        # wrap-ignore
        MatchIterator end()
