from libcpp.string  cimport string as libcpp_utf8_string

cdef extern from "index/index_writer.h" namespace "keyvi::index":
    cdef cppclass IndexWriter:
        IndexWriter(libcpp_utf8_string) except+
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+
        void Flush()
