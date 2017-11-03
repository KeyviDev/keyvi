from libcpp.string cimport string as libcpp_string
from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libc.string cimport const_char

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass JsonDictionaryMerger:
        JsonDictionaryMerger() except +
        JsonDictionaryMerger(size_t memory_limit) except + # DEPRECATED
        JsonDictionaryMerger(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        JsonDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void Merge(libcpp_utf8_string) nogil

cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryMerger:
        CompletionDictionaryMerger() except +
        CompletionDictionaryMerger(size_t memory_limit) except + # DEPRECATED
        CompletionDictionaryMerger(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        CompletionDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void Merge(libcpp_utf8_string) nogil

cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass StringDictionaryMerger:
        StringDictionaryMerger() except +
        StringDictionaryMerger(size_t memory_limit) except + # DEPRECATED
        StringDictionaryMerger(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        StringDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void Merge(libcpp_utf8_string) nogil

cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass KeyOnlyDictionaryMerger:
        KeyOnlyDictionaryMerger() except +
        KeyOnlyDictionaryMerger(size_t memory_limit) except + # DEPRECATED
        KeyOnlyDictionaryMerger(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        KeyOnlyDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void Merge(libcpp_utf8_string) nogil

cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass IntDictionaryMerger:
        IntDictionaryMerger() except +
        IntDictionaryMerger(size_t memory_limit) except + # DEPRECATED
        IntDictionaryMerger(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        IntDictionaryMerger(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void Merge(libcpp_utf8_string) nogil
