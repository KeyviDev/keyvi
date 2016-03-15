cdef extern from "<memory>" namespace "std":
    cppclass shared_ptr[T]:
        shared_ptr()
        shared_ptr(T*)
        void reset()
        T* get() nogil
        int unique()
        int use_count()