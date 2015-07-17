add_library(kdtree_common app_params.cpp app_params.h)
set_target_properties(kdtree_common PROPERTIES FOLDER tpie/apps)

add_executable(build_btree build_btree.cpp)
set_target_properties(build_btree PROPERTIES FOLDER tpie/apps)
add_executable(build_kdbtree build_kdbtree.cpp)
set_target_properties(build_kdbtree PROPERTIES FOLDER tpie/apps)
add_executable(build_bkdtree build_bkdtree.cpp)
set_target_properties(build_bkdtree PROPERTIES FOLDER tpie/apps)
add_executable(datagen datagen.cpp)
set_target_properties(datagen PROPERTIES FOLDER tpie/apps)

target_link_libraries(build_btree tpie kdtree_common)
target_link_libraries(build_kdbtree tpie kdtree_common)
target_link_libraries(build_bkdtree tpie kdtree_common)
target_link_libraries(datagen tpie kdtree_common)

