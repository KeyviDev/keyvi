set_property(GLOBAL PROPERTY USE_FOLDERS On)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sysinfo.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp" @ONLY)

set (HEADERS
		access_type.h
		backtrace.h
		blocks/block.h
		blocks/block_collection.h
		blocks/block_collection_cache.h
		blocks/freespace_collection.h
		btree/base.h
		btree/internal_store.h
		btree/external_store.h
		btree/external_store_base.h
		btree/node.h
		btree/btree.h
        btree/btree_builder.h
		cache_hint.h
		comparator.h
		compressed/buffer.h
		compressed/direction.h
		compressed/predeclare.h
		compressed/request.h
		compressed/scheme.h
		compressed/stream.h
		compressed/thread.h
		config.h.cmake
		cpu_timer.h
		deprecated.h
		disjoint_sets.h
		exception.h
		err.h
		file.h
		file_base.h
		file_base_crtp.h
		file_manager.h
		file_stream.h
		file_stream_base.h
		file_accessor/byte_stream_accessor.h
		file_accessor/file_accessor.h
		file_accessor/stream_accessor.h
		file_accessor/stream_accessor_base.h
		file_accessor/stream_accessor_base.inl
		execution_time_predictor.h
		internal_sort.h
		internal_queue.h
		internal_stack.h
		internal_vector.h
		internal_stack_vector_base.h
		job.h
		loglevel.h
		logstream.h
		mergeheap.h
		merge_sorted_runs.h
		memory.h
		persist.h
		pipelining/buffer.h
		pipelining/container.h
		pipelining/exception.h
		pipelining/factory_base.h
		pipelining/factory_helpers.h
		pipelining/file_stream.h
		pipelining/helpers.h
		pipelining/join.h
		pipelining/maintain_order_type.h
		pipelining/merge.h
		pipelining/merge_sorter.h
		pipelining/merger.h
		pipelining/node.h
		pipelining/node_map_dump.h
		pipelining/node_name.h
		pipelining/numeric.h
		pipelining/pair_factory.h
		pipelining/parallel.h
		pipelining/parallel/aligned_array.h
		pipelining/parallel/base.h
		pipelining/parallel/factory.h
		pipelining/parallel/options.h
		pipelining/parallel/pipes.h
		pipelining/parallel/worker_state.h
		pipelining/pipe_base.h
		pipelining/pipeline.h
		pipelining/reverse.h
		pipelining/serialization_sort.h
		pipelining/sort.h
		pipelining/std_glue.h
		pipelining/stdio.h
		pipelining/tokens.h
		pipelining/uniq.h
		pipelining/virtual.h
		portability.h
		internal_priority_queue.h
		priority_queue.inl
		priority_queue.h
		pq_overflow_heap.h
		pq_overflow_heap.inl
		pq_merge_heap.h
		pq_merge_heap.inl
		fractional_progress.h
		parallel_sort.h
		dummy_progress.h
		progress_indicator_subindicator.h
		progress_indicator_arrow.h
		progress_indicator_base.h
		progress_indicator_spin.h
		progress_indicator_null.h
		progress_indicator_terminal.h
		queue.h
		resource_manager.h
		resources.h
		serialization.h
		serialization2.h
		serialization_stream.h
		serialization_sorter.h
		sort.h
		sort_deprecated.h
		sort_manager.h
		stack.h
		stream.h
		stream_crtp.h
		stream_header.h
		stream_usage.h
		sysinfo.h
		tpie_assert.h
		tpie_log.h
		stats.h
		types.h
		tempname.h
		uncompressed_stream.h
		unused.h
		util.h
		array.h
		bit_array.h
		packed_array.h
		array_view_base.h
		array_view.h
		hash_map.h
		hash.h
		prime.h
		concepts.h
		concept_doc.h
		tpie.h
		unittest.h
		maybe.h
		tiny.h
		)

set (SOURCES
	backtrace.cpp
	blocks/block_collection.cpp
	blocks/block_collection_cache.cpp
	btree/external_store_base.cpp
	compressed/buffer.cpp
	compressed/request.cpp
	compressed/scheme_none.cpp
	compressed/scheme_snappy.cpp
	compressed/stream_base.cpp
	compressed/thread.cpp
	cpu_timer.cpp
	file_base.cpp
	file_manager.cpp
	file_stream_base.cpp
	execution_time_predictor.cpp
	fractional_progress.cpp
	job.cpp
	logstream.cpp
	memory.cpp
	pipelining/merge_sorter.cpp
	pipelining/node.cpp
	pipelining/node_name.cpp
	pipelining/pipeline.cpp
	pipelining/runtime.cpp
	pipelining/tokens.cpp
	portability.cpp
	prime.cpp
	progress_indicator_base.cpp
	progress_indicator_subindicator.cpp
	resource_manager.cpp
	resources.cpp
	serialization_stream.cpp
	hash.cpp
	tempname.cpp
	tpie.cpp
	tpie_log.cpp
	stats.cpp
	util.cpp
	unittest.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp"
	)

if (WIN32)
set (HEADERS ${HEADERS} file_accessor/win32.h file_accessor/win32.inl)
else(WIN32)
set (HEADERS ${HEADERS} file_accessor/posix.h file_accessor/posix.inl)
endif(WIN32)

add_library(tpie ${HEADERS} ${SOURCES})
target_link_libraries(tpie ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(tpie PROPERTIES FOLDER tpie)

if(TPIE_HAS_SNAPPY)
	target_link_libraries(tpie ${Snappy_LIBRARY})
endif(TPIE_HAS_SNAPPY)

install(TARGETS tpie
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
