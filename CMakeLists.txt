cmake_minimum_required (VERSION 2.8.11)
project (datachannels)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Crc32c REQUIRED)

INCLUDE(CheckCXXSourceCompiles)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
check_cxx_source_compiles("
#include <cstdlib>
int main() { int* p = static_cast<int*>(std::aligned_alloc(1024, 1024*sizeof *p)); std::free(p); return 0; }
"
DHAVE_STD_ALIGNED_ALLOC)
if(DHAVE_STD_ALIGNED_ALLOC)
	add_compile_options("-DHAVE_STD_ALIGNED_ALLOC")
endif(DHAVE_STD_ALIGNED_ALLOC)

add_library(libdatachannels "")
set_property(TARGET libdatachannels PROPERTY CXX_STANDARD 17)
target_include_directories (libdatachannels PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory (src)
add_subdirectory (gtests)

