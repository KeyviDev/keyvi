# same import style as autowrap
from match cimport Match as _Match

cdef extern from "keyvi/dictionary/match_iterator.h" namespace "keyvi::dictionary":
    cdef cppclass MatchIterator:
        # wrap-ignore
        _Match operator*()
        # wrap-ignore
        MatchIterator& operator++()
        bint operator==(MatchIterator)
        bint operator!=(MatchIterator)

cdef extern from "keyvi/dictionary/match_iterator.h" namespace "keyvi::dictionary::MatchIterator":
    cdef cppclass MatchIteratorPair:
        # wrap-ignore
        MatchIterator begin()
        # wrap-ignore
        MatchIterator end()
