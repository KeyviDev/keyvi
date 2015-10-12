from libcpp.string cimport string as libcpp_string
from dictionary cimport Dictionary
from  smart_ptr cimport shared_ptr

cdef extern from "compression/predictive_compression.h" namespace "keyvi::compression":
    cdef cppclass PredictiveCompression:
        PredictiveCompression(libcpp_string) except +
        libcpp_string Compress(libcpp_string) nogil
        libcpp_string Uncompress(libcpp_string) nogil
