from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp cimport bool
from match cimport Match

cdef extern from "index/index.h" namespace "keyvi::index":
    cdef cppclass Index:
        Index(libcpp_utf8_string) # wrap-ignore
        Index(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) # wrap-ignore
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+
        void Delete(libcpp_utf8_string) except+
        void Flush() except+
        void FlushAsync() except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
