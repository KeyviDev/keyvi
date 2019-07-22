from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map

cdef extern from "keyvi/vector/vector_types.h" namespace "keyvi::vector":
    cdef cppclass JsonVectorGenerator:
        JsonVectorGenerator() except +
        JsonVectorGenerator(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void PushBack(libcpp_utf8_string) # wrap-ignore
        void SetManifest(libcpp_utf8_string)
        void WriteToFile(libcpp_utf8_string) except +

cdef extern from "keyvi/vector/vector_types.h" namespace "keyvi::vector":
    cdef cppclass StringVectorGenerator:
        StringVectorGenerator() except +
        StringVectorGenerator(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void PushBack(libcpp_utf8_string)
        void SetManifest(libcpp_utf8_string)
        void WriteToFile(libcpp_utf8_string) except +
