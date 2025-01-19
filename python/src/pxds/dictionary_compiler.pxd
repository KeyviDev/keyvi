from libcpp.string cimport string as libcpp_string
from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.map cimport map as libcpp_map
from libcpp.vector cimport vector as libcpp_vector

ctypedef void (*callback_t)(size_t a, size_t b, void* user_data)
        
cdef extern from "keyvi/dictionary/dictionary_types.h" namespace "keyvi::dictionary":
    cdef cppclass CompletionDictionaryCompiler:
        CompletionDictionaryCompiler() except +
        CompletionDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, int) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass FloatVectorDictionaryCompiler:
        FloatVectorDictionaryCompiler() except +
        FloatVectorDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_vector[float]) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass IntDictionaryCompiler:
        IntDictionaryCompiler() except +
        IntDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, long) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass KeyOnlyDictionaryCompiler:
        KeyOnlyDictionaryCompiler() except +
        KeyOnlyDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass JsonDictionaryCompiler:
        JsonDictionaryCompiler() except +
        JsonDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass StringDictionaryCompiler:
        StringDictionaryCompiler() except +
        StringDictionaryCompiler(libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string, libcpp_utf8_string) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyCompletionDictionaryCompiler:
        SecondaryKeyCompletionDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyCompletionDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, int the_value) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyFloatVectorDictionaryCompiler:
        SecondaryKeyFloatVectorDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyFloatVectorDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, libcpp_vector[float] the_value) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyIntDictionaryCompiler:
        SecondaryKeyIntDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyIntDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, long the_value) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyKeyOnlyDictionaryCompiler:
        SecondaryKeyKeyOnlyDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyKeyOnlyDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyJsonDictionaryCompiler:
        SecondaryKeyJsonDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyJsonDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, libcpp_utf8_string the_value) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file

    cdef cppclass SecondaryKeyStringDictionaryCompiler:
        SecondaryKeyStringDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys) except +
        SecondaryKeyStringDictionaryCompiler(libcpp_vector[libcpp_utf8_string] secondary_keys, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] value_store_params) except +
        void Add(libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, libcpp_utf8_string the_value) except + # wrap-as:add
        void Compile() nogil # wrap-ignore
        void Compile(callback_t, void*) nogil # wrap-ignore
        void SetManifest(libcpp_utf8_string) except + # wrap-as:set_manifest
        void WriteToFile(libcpp_utf8_string) except + # wrap-as:write_to_file
