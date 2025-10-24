from libc.stdint cimport uint32_t
from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libcpp.string cimport string as libcpp_utf8_output_string
from libcpp cimport bool
from cpython.ref cimport PyObject
from compression cimport CompressionAlgorithm

cdef extern from "keyvi/dictionary/match.h" namespace "keyvi::dictionary":
    cdef cppclass Match:
        Match()
        size_t GetStart() # wrap-ignore
        void SetStart(size_t start) # wrap-ignore
        size_t GetEnd() # wrap-ignore
        void SetEnd(size_t end) # wrap-ignore
        float GetScore() # wrap-ignore
        void SetScore(float score) # wrap-ignore
        uint32_t GetWeight() # wrap-ignore
        libcpp_utf8_output_string GetMatchedString() # wrap-ignore
        void SetMatchedString (libcpp_utf8_string matched_string) # wrap-ignore
        PyObject* GetAttributePy(libcpp_utf8_string) except + nogil # wrap-ignore
        libcpp_utf8_output_string GetValueAsString() except + # wrap-as:value_as_string
        libcpp_string GetRawValueAsString() except + # wrap-as:raw_value_as_string
        libcpp_string GetMsgPackedValueAsString() except + # wrap-as:msgpacked_value_as_string
        libcpp_string GetMsgPackedValueAsString(CompressionAlgorithm) except + # wrap-as:msgpacked_value_as_string
        void SetRawValue(libcpp_utf8_string) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, libcpp_utf8_string) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, float) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, int) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, bool) except + # wrap-ignore
        bool IsEmpty() # wrap-ignore

