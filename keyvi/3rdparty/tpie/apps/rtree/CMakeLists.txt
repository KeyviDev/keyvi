SET (COMMON_DEPS
	hilbert.cpp
	hilbert.h
	app_config.h
	bulkloader.h
	rectangle.h
	rectangle_comparators.h
	rstarnode.h
	rstarnode_info.h
	rstartree.h
	scan_boundingbox.h
	)
add_library(rtree_common ${COMMON_DEPS})
set_target_properties(rtree_common PROPERTIES FOLDER tpie/apps)

add_executable(ascii2stream ascii2stream.cpp)
set_target_properties(ascii2stream PROPERTIES FOLDER tpie/apps)
add_executable(build_rtree build_rtree.cpp)
set_target_properties(build_rtree PROPERTIES FOLDER tpie/apps)
add_executable(mbr mbr.cpp)
set_target_properties(mbr PROPERTIES FOLDER tpie/apps)
add_executable(test_rtree test_rtree.cpp)
set_target_properties(test_rtree PROPERTIES FOLDER tpie/apps)

target_link_libraries(build_rtree tpie rtree_common)
target_link_libraries(test_rtree tpie rtree_common)
target_link_libraries(ascii2stream tpie rtree_common)
target_link_libraries(mbr tpie rtree_common)


