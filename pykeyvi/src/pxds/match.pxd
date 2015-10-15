from libc.stdint cimport uint32_t
from libcpp.string cimport string as libcpp_string
from libcpp cimport bool
from cpython.ref cimport PyObject

cdef extern from "dictionary/match.h" namespace "keyvi::dictionary":
    cdef cppclass Match:
        Match()
        Match(Match& m)
        size_t GetStart()
        void SetStart(size_t start)
        size_t GetEnd()
        void SetEnd(size_t end)
        float GetScore()
        void SetScore(float score)
        libcpp_string GetMatchedString()
        void SetMatchedString (libcpp_string matched_string)
        PyObject* GetAttributePy(libcpp_string) nogil except + # wrap-ignore
        libcpp_string GetValueAsString()
        libcpp_string GetRawValueAsString()
        void SetAttribute(libcpp_string, libcpp_string) except + # wrap-ignore
        void SetAttribute(libcpp_string, float) except + # wrap-ignore
        void SetAttribute(libcpp_string, int) except + # wrap-ignore
        void SetAttribute(libcpp_string, bool) except + # wrap-ignore
        bool IsEmpty()

cdef extern from "dictionary/fsa/internal/json_value_store.h" namespace "keyvi::dictionary::fsa::internal":
    cdef cppclass JsonValueStoreReader:
        @staticmethod
        libcpp_string DecodeValue(libcpp_string&)
