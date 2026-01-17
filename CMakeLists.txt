set(CMAKE_CXX_STANDARD 23)
cmake_minimum_required(VERSION 3.10)
project(enbt)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(enbt ${SOURCES})

include_directories("include")
include_directories("include/thirdparty")
target_link_libraries(enbt)

add_subdirectory(tests)
