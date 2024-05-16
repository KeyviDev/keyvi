from cython.operator cimport dereference, preincrement
cimport cython.operator as co

# same import style as autowrap
from match cimport Match as _Match
from match_iterator cimport MatchIterator as _MatchIterator

cdef class MatchIterator:
    cdef _MatchIterator it
    cdef _MatchIterator end

    def __iter__(self):
        return self

    def __next__(self):
        #if  co.dereference( self.it ) == co.dereference( self.end ) :
        if  self.it == self.end:

            raise StopIteration()
        cdef _Match * _r = new _Match(co.dereference( self.it ))
        with nogil:
            co.preincrement( self.it )
        
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = shared_ptr[_Match](_r)

        return py_result

    def set_min_weight(self, w):
        self.it.SetMinWeight(w)
