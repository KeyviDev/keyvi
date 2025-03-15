cdef extern from "keyvi/compression/compression_strategy.h" namespace "keyvi::compression":
    ctypedef enum CompressionAlgorithm:
        NO_COMPRESSION,
        ZLIB_COMPRESSION,
        SNAPPY_COMPRESSION
