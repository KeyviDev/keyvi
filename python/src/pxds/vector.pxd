from libcpp.string cimport string as libcpp_utf8_output_string

cdef extern from "keyvi/vector/vector_types.h" namespace "keyvi::vector":
    cdef cppclass JsonVector:
        JsonVector(libcpp_utf8_output_string filename) except +
        libcpp_utf8_output_string Get(size_t index) # wrap-ignore
        size_t Size() # wrap-as:__len__
        libcpp_utf8_output_string Manifest() # wrap-as:manifest

cdef extern from "keyvi/vector/vector_types.h" namespace "keyvi::vector":
    cdef cppclass StringVector:
        StringVector(libcpp_utf8_output_string filename) except +
        libcpp_utf8_output_string Get(size_t index) # wrap-as:__getitem__
        size_t Size() # wrap-as:__len__
        libcpp_utf8_output_string Manifest() # wrap-as:manifest
