SET (COMMON_DEPS
    app_config.h
    app_config.cpp
    fill_upper_tri.h
    fill_value.h
    getopts.h
    getopts.cpp
    matrix.h
    matrix_fill.h
    matrix_blocks.h
    matrix_blocks.cpp
    matrix_pad.h
    mm_matrix.h
    mm_matrix_base.h
    mm_submatrix.h
    mm_rowref.h
    mm_colref.h
    parse_args.h
    parse_args.cpp
    scan_count.h
    scan_count.cpp
    scan_uniform_sm.h
    scan_uniform_sm.cpp
    sparse_matrix.h
)

add_library(matrix_common ${COMMON_DEPS})
set_target_properties(matrix_common PROPERTIES FOLDER tpie/apps)

add_executable(test_matrix test_matrix.cpp)
set_target_properties(test_matrix PROPERTIES FOLDER tpie/apps)
add_executable(test_sparse_matrix test_sparse_matrix.cpp)
set_target_properties(test_sparse_matrix PROPERTIES FOLDER tpie/apps)
add_executable(test_matrix_pad test_matrix_pad.cpp)
set_target_properties(test_matrix_pad PROPERTIES FOLDER tpie/apps)

target_link_libraries(test_matrix tpie matrix_common)
target_link_libraries(test_sparse_matrix tpie matrix_common)
target_link_libraries(test_matrix_pad tpie matrix_common)



