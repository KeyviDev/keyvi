#Path to output doxygen configuration file
SET(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#Search for doxygen
find_package(Doxygen)

if (DOXYGEN)
#If Doxygen is present, add docs custom target
#that executes doxygen

option(INTERNAL_DOCS "Enable internal documentation comments in the doxygen output" ON)

#This variable is automatically replaced when configuring Doxyfile.in.
if (INTERNAL_DOCS)
	set(USE_INTERNAL_DOCS "YES")
else (INTERNAL_DOCS)
	set(USE_INTERNAL_DOCS "NO")
endif(INTERNAL_DOCS)

ADD_CUSTOM_TARGET(apidocs
	sh -c "sed -e \"s/\@VERSION\@/`git describe --always`/g\" ${PROJECT_SOURCE_DIR}/doc/header.html > ${PROJECT_BINARY_DIR}/doc/header.html"
	COMMAND sh -c "sed -e \"s/\@VERSION\@/`git describe --always`/g\" ${PROJECT_SOURCE_DIR}/doc/footer.html > ${PROJECT_BINARY_DIR}/doc/footer.html"
	COMMAND ${DOXYGEN} ${DOXYFILE}
	VERBATIM)

file(READ code/btree_key.inl DOCCODE_BTREE_KEY)
file(READ code/btree_augment.inl DOCCODE_BTREE_AUGMENT)
configure_file(btree.dox.in btree.dox @ONLY)

#CMake should process the Doxyfile.in file replacing
#variables placing the result in the build directory
configure_file(Doxyfile.in ${DOXYFILE} @ONLY)


file(READ code/file_stream.inl DOCCODE_FILE_STREAM)
configure_file(file_stream.dox.in file_stream.dox @ONLY)

file(READ code/fractiondb.inl DOCCODE_FRACTIONDB)
configure_file(fractiondb.dox.in fractiondb.dox @ONLY)

file(READ code/memory.inl DOCCODE_MEMORY)
file(READ code/memory2.inl DOCCODE_MEMORY2)
configure_file(memory.dox.in memory.dox @ONLY)

file(READ code/priority_queue.inl DOCCODE_PRIORITY_QUEUE)
configure_file(priority_queue.dox.in priority_queue.dox @ONLY)

file(READ code/progress1.inl DOCCODE_PROGRESS1)
file(READ code/progress2.inl DOCCODE_PROGRESS2)
file(READ code/progress3.inl DOCCODE_PROGRESS3)
configure_file(progress.dox.in progress.dox @ONLY)

file(READ code/sorting_external.inl DOCCODE_SORTING_EXTERNAL)
configure_file(sorting_external.dox.in sorting_external.dox @ONLY)

file(READ code/sorting_internal.inl DOCCODE_SORTING_INTERNAL)
configure_file(sorting_internal.dox.in sorting_internal.dox @ONLY)

configure_file(queue.dox.in queue.dox @ONLY)

file(READ code/serialization.inl DOCCODE_SERIALIZATION)
configure_file(serialization.dox.in serialization.dox @ONLY)

else (DOXYGEN)
    message(STATUS "Doxygen not found, API documentation cannot be generated.") 
endif(DOXYGEN)

add_executable(doccode code/code.cpp)
target_link_libraries(doccode tpie ${Boost_LIBRARIES})

configure_file(madalgo-doxygen.css html/madalgo-doxygen.css COPYONLY)
configure_file(printstyle.css html/printstyle.css COPYONLY)
configure_file(pipelining_memory.png html/pipelining_memory.png COPYONLY)
configure_file(pipelining_sequence.png html/pipelining_sequence.png COPYONLY)
configure_file(pipelining_sequence_pull.png html/pipelining_sequence_pull.png COPYONLY)
