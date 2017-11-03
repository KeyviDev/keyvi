from libcpp.string cimport string as libcpp_string
from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryCompiler:
        CompletionDictionaryCompiler() except +
        CompletionDictionaryCompiler(size_t memory_limit) except + # DEPRECATED
        CompletionDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        CompletionDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, int) except +
        void __setitem__ (libcpp_utf8_string, int) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass IntDictionaryCompiler:
        IntDictionaryCompiler() except +
        IntDictionaryCompiler(size_t memory_limit) except + # DEPRECATED
        IntDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        IntDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, int) except +
        void __setitem__ (libcpp_utf8_string, int) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass KeyOnlyDictionaryCompiler:
        KeyOnlyDictionaryCompiler() except +
        KeyOnlyDictionaryCompiler(size_t memory_limit) except + # DEPRECATED
        KeyOnlyDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        KeyOnlyDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +
        
    cdef cppclass JsonDictionaryCompiler:
        JsonDictionaryCompiler() except +
        JsonDictionaryCompiler(size_t memory_limit) except + # DEPRECATED
        JsonDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        JsonDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except +
        void __setitem__(libcpp_utf8_string, libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) except + # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +
    
    cdef cppclass JsonDictionaryCompilerSmallData:
        JsonDictionaryCompilerSmallData() except +
        JsonDictionaryCompilerSmallData(size_t memory_limit) except + # DEPRECATED
        JsonDictionaryCompilerSmallData(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        JsonDictionaryCompilerSmallData(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_string, libcpp_string) except + # wrap-ignore
        void __setitem__(libcpp_string, libcpp_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) except + # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +
        
    cdef cppclass StringDictionaryCompiler:
        StringDictionaryCompiler() except +
        StringDictionaryCompiler(size_t memory_limit) except + # DEPRECATED
        StringDictionaryCompiler(size_t memory_limit, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except + # DEPRECATED
        StringDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except +
        void __setitem__(libcpp_utf8_string, libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifestFromString(libcpp_utf8_string) # wrap-ignore
        void WriteToFile(libcpp_utf8_string) except +

