from libcpp.string  cimport string as libcpp_utf8_string
from libcpp cimport bool
from match cimport Match

cdef extern from "index/read_only_index.h" namespace "keyvi::index":
    cdef cppclass ReadOnlyIndex:
        ReadOnlyIndex(libcpp_utf8_string) except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore