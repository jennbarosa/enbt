enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/thirdparty)

add_executable(enbt_parse_test ${CMAKE_SOURCE_DIR}/tests/test_parse.cpp ${CMAKE_SOURCE_DIR}/src/parse.cpp)
add_test(NAME enbt_parsing COMMAND enbt_parse_test)
