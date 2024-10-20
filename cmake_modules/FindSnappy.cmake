# Snappy, a fast compressor/decompressor

find_path(Snappy_INCLUDE_DIR NAMES snappy.h)

find_library(Snappy_LIBRARY NAMES snappy)

message("${Snappy_INCLUDE_DIR} -- ${Snappy_LIBRARY}")

include(SelectLibraryConfigurations)
SELECT_LIBRARY_CONFIGURATIONS(SNAPPY)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Snappy DEFAULT_MSG
    Snappy_LIBRARY Snappy_INCLUDE_DIR
)

mark_as_advanced(Snappy_INCLUDE_DIR Snappy_LIBRARY)