from libcpp.string cimport string
from libc.string cimport const_char


cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass KeyOnlyDictionaryGenerator:
        KeyOnlyDictionaryGenerator() except +
        void Add(const_char*) except +
        void CloseFeeding()
        void WriteToFile(const_char*)