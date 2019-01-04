target_include_directories (libdatachannels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/internal)
target_include_directories (libdatachannels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(libdatachannels PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/Association.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PacketHeader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Stream.cpp
)

add_subdirectory (chunks)
