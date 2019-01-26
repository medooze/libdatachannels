cmake_minimum_required (VERSION 2.8.11)
project (datachannels)

add_library(libdatachannels "")
set_property(TARGET libdatachannels PROPERTY CXX_STANDARD 17)
target_include_directories (libdatachannels PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_subdirectory (src)
add_subdirectory (gtests)

#add_subdirectory (gtests)
