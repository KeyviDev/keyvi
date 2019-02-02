# Snappy, a fast compressor/decompressor

include(LibFindMacros)

find_path(Snappy_INCLUDE_DIR
	NAMES snappy.h
)

find_library(Snappy_LIBRARY
	NAMES snappy
)

set(Snappy_PROCESS_INCLUDES Snappy_INCLUDE_DIR)
set(Snappy_PROCESS_LIBS Snappy_LIBRARY)

libfind_process(Snappy)
