from libc.stdint cimport uint32_t
from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libcpp.string cimport string as libcpp_utf8_output_string
from libcpp cimport bool
from cpython.ref cimport PyObject

cdef extern from "keyvi/dictionary/match.h" namespace "keyvi::dictionary":
    cdef cppclass Match:
        Match()
        Match(Match& m)
        size_t GetStart()
        void SetStart(size_t start)
        size_t GetEnd()
        void SetEnd(size_t end)
        float GetScore()
        void SetScore(float score)
        libcpp_utf8_output_string GetMatchedString()
        void SetMatchedString (libcpp_utf8_string matched_string)
        PyObject* GetAttributePy(libcpp_utf8_string) nogil except + # wrap-ignore
        libcpp_utf8_output_string GetValueAsString() except +
        libcpp_string GetRawValueAsString() except +
        libcpp_string GetMsgPackedValueAsString() except + # wrap-ignore
        void SetRawValue(libcpp_utf8_string) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, libcpp_utf8_string) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, float) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, int) except + # wrap-ignore
        void SetAttribute(libcpp_utf8_string, bool) except + # wrap-ignore
        bool IsEmpty()

