from libcpp.string cimport string as libcpp_string
from libcpp.map cimport map as libcpp_map
from libc.string cimport const_char

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryCompiler:
        CompletionDictionaryCompiler() except +
        CompletionDictionaryCompiler(size_t memory_limit) except +
        void Add(const_char*, int) except +
        void __setitem__ (const_char*, int) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(const_char*) # wrap-ignore
        void WriteToFile(const_char*)
        
    cdef cppclass KeyOnlyDictionaryCompiler:
        KeyOnlyDictionaryCompiler() except +
        KeyOnlyDictionaryCompiler(size_t memory_limit) except +
        void Add(const_char*) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(const_char*) # wrap-ignore
        void WriteToFile(const_char*)
        
    cdef cppclass JsonDictionaryCompiler:
        JsonDictionaryCompiler() except + # wrap-ignore
        JsonDictionaryCompiler(size_t memory_limit) except + # wrap-ignore
        JsonDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_string, libcpp_string]& value_store_params) except + # wrap-ignore
        void Add(const_char*, const_char*) except +
        void __setitem__(const_char*, const_char*) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(const_char*) # wrap-ignore
        void WriteToFile(const_char*)
    
    cdef cppclass StringDictionaryCompiler:
        StringDictionaryCompiler() except +
        StringDictionaryCompiler(size_t memory_limit) except +
        void Add(const_char*, const_char*) except +
        void __setitem__(const_char*, const_char*) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(const_char*) # wrap-ignore
        void WriteToFile(const_char*)

