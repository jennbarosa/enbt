enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/thirdparty)

# Original parse test
add_executable(enbt_parse_test ${CMAKE_SOURCE_DIR}/tests/test_parse.cpp ${CMAKE_SOURCE_DIR}/src/parse.cpp ${CMAKE_SOURCE_DIR}/src/NBTReader.cpp ${CMAKE_SOURCE_DIR}/src/NBTWriter.cpp)
add_test(NAME enbt_parsing COMMAND enbt_parse_test)

# NBT Reader tests
add_executable(enbt_nbt_reader_test ${CMAKE_SOURCE_DIR}/tests/test_nbt_reader.cpp ${CMAKE_SOURCE_DIR}/src/NBTReader.cpp ${CMAKE_SOURCE_DIR}/src/NBTWriter.cpp)
add_test(NAME enbt_nbt_reader COMMAND enbt_nbt_reader_test)

# Serialization tests
add_executable(enbt_serialization_test ${CMAKE_SOURCE_DIR}/tests/test_serialization.cpp ${CMAKE_SOURCE_DIR}/src/serialize.cpp ${CMAKE_SOURCE_DIR}/src/parse.cpp ${CMAKE_SOURCE_DIR}/src/NBTReader.cpp ${CMAKE_SOURCE_DIR}/src/NBTWriter.cpp)
add_test(NAME enbt_serialization COMMAND enbt_serialization_test)

# Reverse conversion integration tests
add_executable(enbt_reverse_test ${CMAKE_SOURCE_DIR}/tests/test_reverse_conversion.cpp ${CMAKE_SOURCE_DIR}/src/parse.cpp ${CMAKE_SOURCE_DIR}/src/serialize.cpp ${CMAKE_SOURCE_DIR}/src/NBTReader.cpp ${CMAKE_SOURCE_DIR}/src/NBTWriter.cpp)
add_test(NAME enbt_reverse_conversion COMMAND enbt_reverse_test)
