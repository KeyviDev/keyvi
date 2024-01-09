from libcpp.string cimport string as libcpp_string
from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp.vector cimport vector as libcpp_vector

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryCompiler:
        CompletionDictionaryCompiler() except +
        CompletionDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, int) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass FloatVectorDictionaryCompiler:
        FloatVectorDictionaryCompiler() except +
        FloatVectorDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_vector[float]) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass IntDictionaryCompiler:
        IntDictionaryCompiler() except +
        IntDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, long) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass IntDictionaryCompilerSmallData:
        IntDictionaryCompilerSmallData() except +
        IntDictionaryCompilerSmallData(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, long) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass KeyOnlyDictionaryCompiler:
        KeyOnlyDictionaryCompiler() except +
        KeyOnlyDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass JsonDictionaryCompiler:
        JsonDictionaryCompiler() except +
        JsonDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass JsonDictionaryCompilerSmallData:
        JsonDictionaryCompilerSmallData() except +
        JsonDictionaryCompilerSmallData(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_string, libcpp_string) except + # wrap-ignore
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

    cdef cppclass StringDictionaryCompiler:
        StringDictionaryCompiler() except +
        StringDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except +
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except +
        void WriteToFile(libcpp_utf8_string) except +

