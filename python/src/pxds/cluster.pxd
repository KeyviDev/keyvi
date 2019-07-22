from libcpp.string cimport string as libcpp_utf8_string
from libc.stdint cimport uint32_t

cdef extern from "keyvi/dictionary/util/jump_consistent_hash.h" namespace "keyvi::dictionary::util":
    uint32_t JumpConsistentHashString(libcpp_utf8_string, uint32_t)
