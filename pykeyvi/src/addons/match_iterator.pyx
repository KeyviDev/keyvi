from cython.operator cimport dereference, preincrement
cimport cython.operator as co

# same import style as autowrap
from match cimport Match as _Match
from match_iterator cimport MatchIterator as _MatchIterator

cdef class MatchIterator:
    cdef _MatchIterator it
    cdef _MatchIterator end

    #def __cinit__(self, ):
    #    self.end = new _MatchIterator()

    # Most likely, you will be calling this directly from this
    # or another Cython module, not from Python.
    #cdef set_iter(self, _MatchIterator it):
    #    self.it = it

    def __iter__(self):
        return self

    #def __dealloc__(self):
        # This works by calling "delete" in C++, you should not
        # fear that Cython will call "free"
    #    del self.it
    #    del self.end

    def __next__(self):
        # This works correctly by using "*it" and "*end" in the code,
        #if  co.dereference( self.it ) == co.dereference( self.end ) :
        if  self.it == self.end:

            raise StopIteration()
        cdef _Match * _r = new _Match(co.dereference( self.it ))

        # This also does the expected thing.
        co.preincrement( self.it )

        cdef Match py_result = Match.__new__(Match)
        py_result.inst = shared_ptr[_Match](_r)

        return py_result
