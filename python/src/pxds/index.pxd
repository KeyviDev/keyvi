from libcpp.string  cimport string as libcpp_utf8_string

cdef extern from "index/index.h" namespace "keyvi::index":
    cdef cppclass Index:
        Index(libcpp_utf8_string) except+
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+
        void Flush()
