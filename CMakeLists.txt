cmake_minimum_required(VERSION 3.22)
project(Interpreter)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/BuildFlags.cmake)
include(cmake/TestSolution.cmake)

include(FetchContent)

find_package(Catch REQUIRED)

add_subdirectory(allocations_checker)

set(INTERPRETER_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

set(INTERPRETER_TESTS
        # from tokenizer
        tests/test_tokenizer.cpp

        # from parser
        tests/test_parser.cpp

        # from basic
        tests/test_boolean.cpp
        tests/test_eval.cpp
        tests/test_integer.cpp
        tests/test_list.cpp
        tests/test_fuzzing_2.cpp

        tests/test_symbol.cpp
        tests/test_pair_mut.cpp
        tests/test_control_flow.cpp
        tests/test_lambda.cpp
        object.cpp
)

add_catch(test_interpreter
        ${INTERPRETER_TESTS})

include(sources.cmake)

target_include_directories(interpreter PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${INTERPRETER_COMMON_DIR})

target_link_libraries(test_interpreter interpreter allocations_checker)
