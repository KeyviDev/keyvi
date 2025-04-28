cdef extern from "keyvi/compression/compression_algorithm.h" namespace "keyvi::compression":
    ctypedef enum CompressionAlgorithm:
        NO_COMPRESSION,
        ZLIB_COMPRESSION,
        SNAPPY_COMPRESSION,
        ZSTD_COMPRESSION
