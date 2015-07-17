from libc.string cimport const_char
from libc.stdint cimport uint32_t

cdef extern from "dictionary/util/jump_consistent_hash.h" namespace "keyvi::dictionary::util":
    uint32_t JumpConsistentHashString(const_char*, uint32_t)