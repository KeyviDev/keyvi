set (COMMON_DEPS
		app_config.h
		app_config.cpp
		getopts.cpp 
		getopts.h
		parse_args.cpp
		parse_args.h
		test_portability.h
	)

add_library(test_common ${COMMON_DEPS})
set_target_properties(test_common PROPERTIES FOLDER tpie/test)

macro(add_tpie_test NAME)
	add_executable(${NAME} ${NAME}.cpp) 
	set_target_properties(${NAME} PROPERTIES FOLDER tpie/test)
	target_link_libraries(${NAME} tpie test_common )
endmacro(add_tpie_test)

add_tpie_test(test_ami_stack)
add_tpie_test(test_sort)
add_tpie_test(test_big_scan)

add_executable(test_pipelining pipelining.cpp)
set_target_properties(test_pipelining PROPERTIES FOLDER tpie/test)
target_link_libraries(test_pipelining tpie )

add_executable(pipelining2 pipelining2.cpp)
set_target_properties(pipelining2 PROPERTIES FOLDER tpie/test)
target_link_libraries(pipelining2 tpie )

add_subdirectory(speed_regression)

add_subdirectory(unit)

add_executable(tpiecat cat.cpp)
set_target_properties(tpiecat PROPERTIES FOLDER tpie/test)
target_link_libraries(tpiecat tpie)

add_executable(sort sort.cpp)
set_target_properties(sort PROPERTIES FOLDER tpie/test)
target_link_libraries(sort tpie)

add_executable(lines lines.cpp)
set_target_properties(lines PROPERTIES FOLDER tpie/test)
target_link_libraries(lines tpie)

add_executable(test_parallel_sort_threshold test_parallel_sort_threshold.cpp)
set_target_properties(test_parallel_sort_threshold PROPERTIES FOLDER tpie/test)
target_link_libraries(test_parallel_sort_threshold tpie)

add_executable(test_job_overhead test_job_overhead.cpp)
set_target_properties(test_job_overhead PROPERTIES FOLDER tpie/test)
target_link_libraries(test_job_overhead tpie)

add_executable(atomic_stats test_atomic_stats.cpp)
set_target_properties(atomic_stats PROPERTIES FOLDER tpie/test)
target_link_libraries(atomic_stats tpie ${Boost_LIBRARIES})
