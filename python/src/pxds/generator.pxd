from libcpp.string cimport string as libcpp_utf8_string
from libc.string cimport const_char


cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass KeyOnlyDictionaryGenerator:
        KeyOnlyDictionaryGenerator() except +
        void Add(libcpp_utf8_string) except + # wrap-as:add
        void CloseFeeding() # wrap-as:close_feeding
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file
