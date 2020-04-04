from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from dictionary cimport Dictionary
from std_smart_ptr cimport shared_ptr

cdef extern from "keyvi/transform/fsa_transform.h" namespace "keyvi::transform":
    cdef cppclass FsaTransform:
        FsaTransform(shared_ptr[Dictionary]) except +
        libcpp_string Normalize(libcpp_utf8_string) nogil
