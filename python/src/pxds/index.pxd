from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp.vector cimport vector as libcpp_vector
from libcpp.pair  cimport pair  as libcpp_pair
from libcpp cimport bool
from match cimport Match
from std_smart_ptr cimport shared_ptr as s_shared_ptr

cdef extern from "keyvi/index/index.h" namespace "keyvi::index":
    cdef cppclass Index:
        Index(libcpp_utf8_string) # wrap-ignore
        Index(libcpp_utf8_string, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] params) # wrap-ignore
        void Set(libcpp_utf8_string, libcpp_utf8_string) except+
        void MSet(s_shared_ptr[libcpp_vector[libcpp_pair[libcpp_utf8_string, libcpp_utf8_string]]]) # wrap-ignore
        void Delete(libcpp_utf8_string) except+
        void Flush() except+
        void Flush(bool) except+
        bool Contains(libcpp_utf8_string) # wrap-ignore
        Match operator[](libcpp_utf8_string) # wrap-ignore
