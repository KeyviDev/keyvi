from libc.stdint cimport uint32_t
from cpython.version cimport PY_MAJOR_VERSION


def JumpConsistentHashString(key,  num_buckets):
    """Cython signature: uint32_t JumpConsistentHashString(str, uint32_t)"""

    assert isinstance(num_buckets, (int, long)), 'arg num_buckets wrong type'

    if (PY_MAJOR_VERSION >= 3):
        assert isinstance(key, str), 'arg key wrong type'
        key_bytes = key.encode()
    else:
        assert isinstance(key, (bytes, unicode)), 'arg key wrong type'
        if isinstance(key, unicode):
            key_bytes = key.encode('utf-8')
        else:
            key_bytes = key

    assert isinstance(key_bytes, bytes)
    cdef const_char * input_key_bytes = <const_char *> key_bytes

    cdef uint32_t _r = _JumpConsistentHashString_cluster(input_key_bytes, (<uint32_t>num_buckets))
    py_result = <uint32_t>_r
    return py_result
