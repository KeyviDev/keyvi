from libcpp.string cimport string as libcpp_utf8_string
from libc.string cimport const_char


cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass KeyOnlyDictionaryGenerator:
        KeyOnlyDictionaryGenerator() except +
        void Add(libcpp_utf8_string) except +
        void CloseFeeding()
        void WriteToFile(libcpp_utf8_string) except +
