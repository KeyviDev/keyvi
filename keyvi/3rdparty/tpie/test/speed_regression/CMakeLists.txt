set (SPEED_DEPS testtime.h stat.h)

list(APPEND SPEED_DEPS "${PROJECT_SOURCE_DIR}/tpie/sysinfo.h")

add_executable(btree_speed_test btree.cpp ${SPEED_DEPS})
target_link_libraries(btree_speed_test tpie)
set_target_properties(btree_speed_test PROPERTIES FOLDER tpie/test)

add_executable(compressed_speed_test compressed.cpp ${SPEED_DEPS})
target_link_libraries(compressed_speed_test tpie)
set_target_properties(compressed_speed_test PROPERTIES FOLDER tpie/test)

add_executable(stream_speed_test stream.cpp ${SPEED_DEPS})
target_link_libraries(stream_speed_test tpie)
set_target_properties(stream_speed_test PROPERTIES FOLDER tpie/test)

add_executable(stream_speed_test_2 stream2.cpp ${SPEED_DEPS})
target_link_libraries(stream_speed_test_2 tpie)
set_target_properties(stream_speed_test_2 PROPERTIES FOLDER tpie/test)

add_executable(progress_speed_test progress.cpp ${SPEED_DEPS})
target_link_libraries(progress_speed_test tpie)
set_target_properties(progress_speed_test PROPERTIES FOLDER tpie/test)

add_executable(sort_speed_test sort.cpp ${SPEED_DEPS})
target_link_libraries(sort_speed_test tpie)
set_target_properties(sort_speed_test PROPERTIES FOLDER tpie/test)

add_executable(queue_speed_test queue.cpp ${SPEED_DEPS})
target_link_libraries(queue_speed_test tpie)
set_target_properties(queue_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pq_speed_test priority_queue.cpp ${SPEED_DEPS})
target_link_libraries(pq_speed_test tpie)
set_target_properties(pq_speed_test PROPERTIES FOLDER tpie/test)

add_executable(array_speed_test array.cpp)
target_link_libraries(array_speed_test tpie)
set_target_properties(array_speed_test PROPERTIES FOLDER tpie/test)

add_executable(file_stream_speed_test file_stream.cpp ${SPEED_DEPS})
target_link_libraries(file_stream_speed_test tpie)
set_target_properties(file_stream_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pipeline_speed_test pipelining.cpp)
target_link_libraries(pipeline_speed_test tpie)
set_target_properties(pipeline_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pipelining_sort_test pipelining_sort_test.cpp)
target_link_libraries(pipelining_sort_test tpie)
set_target_properties(pipelining_sort_test PROPERTIES FOLDER tpie/test)

add_executable(serialization_speed serialization.cpp ${SPEED_DEPS})
target_link_libraries(serialization_speed tpie)
set_target_properties(serialization_speed PROPERTIES FOLDER tpie/test)

add_executable(numbergen numbergen.cpp ${SPEED_DEPS})
target_link_libraries(numbergen tpie)
set_target_properties(numbergen PROPERTIES FOLDER tpie/test)

add_executable(fractile_tpie fractile_tpie.cpp ${SPEED_DEPS})
target_link_libraries(fractile_tpie tpie)
set_target_properties(fractile_tpie PROPERTIES FOLDER tpie/test)

add_executable(fractile_serialization fractile_serialization.cpp ${SPEED_DEPS})
target_link_libraries(fractile_serialization tpie)
set_target_properties(fractile_serialization PROPERTIES FOLDER tpie/test)

add_executable(btree_builder btree_builder.cpp ${SPEED_DEPS})
target_link_libraries(btree_builder tpie)
set_target_properties(btree_builder PROPERTIES FOLDER tpie/test)
