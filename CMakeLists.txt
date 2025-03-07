cmake_minimum_required(VERSION 3.10)

project(jerry VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(jerry
  src/main.cpp
  src/Tokenizer.cpp
)

target_include_directories(jerry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    target_compile_options(jerry PRIVATE /W4)
else()
    target_compile_options(jerry PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
find_package(GTest REQUIRED)
add_executable(tokenizer_tests
  test/TokenizerTest.cpp
  src/Tokenizer.cpp
)
target_include_directories(tokenizer_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(tokenizer_tests
  PRIVATE
  GTest::gtest
  GTest::gtest_main
  pthread
)

include(GoogleTest)
gtest_discover_tests(tokenizer_tests)

install(TARGETS jerry DESTINATION bin)
