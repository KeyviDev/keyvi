from libcpp.string cimport string as libcpp_string
from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libc.string cimport const_char

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass JsonDictionaryMerger:
        JsonDictionaryMerger() except +
        JsonDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifest(libcpp_utf8_string) except +
        void Merge(libcpp_utf8_string) nogil

cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryMerger:
        CompletionDictionaryMerger() except +
        CompletionDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifest(libcpp_utf8_string) except +
        void Merge(libcpp_utf8_string) nogil

cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass StringDictionaryMerger:
        StringDictionaryMerger() except +
        StringDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifest(libcpp_utf8_string) except +
        void Merge(libcpp_utf8_string) nogil

cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass KeyOnlyDictionaryMerger:
        KeyOnlyDictionaryMerger() except +
        KeyOnlyDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifest(libcpp_utf8_string) except +
        void Merge(libcpp_utf8_string) nogil

cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass IntDictionaryMerger:
        IntDictionaryMerger() except +
        IntDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifest(libcpp_utf8_string) except +
        void Merge(libcpp_utf8_string) nogil
