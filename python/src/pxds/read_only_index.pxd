from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp cimport bool
from match cimport Match

cdef extern from "keyvi/index/read_only_index.h" namespace "keyvi::index":
    cdef cppclass ReadOnlyIndex:
        ReadOnlyIndex(libcpp_utf8_string) except+
        ReadOnlyIndex(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
