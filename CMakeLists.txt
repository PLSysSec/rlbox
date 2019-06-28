cmake_minimum_required(VERSION 3.11)

project(rlbox-cpp17 VERSION 0.1 DESCRIPTION "RLBox safe sandboxing API in C++17")

########## Project Settings ##########

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

########## Dev Tools ##########

find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
    # Config in .clang-tidy
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE
        ALL_CXX_SOURCE_FILES
        code/*.[chi]pp code/*.[chi]xx code/*.cc code/*.hh code/*.ii code/*.[CHI]
    )

    # Config in .clang-format
    add_custom_target(
        format-source
        COMMAND clang-format
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
    )
endif()

########## Dependencies ##########

include(FetchContent)
FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.9.1
)

FetchContent_GetProperties(catch2)
if (NOT catch2_POLULATED)
    FetchContent_Populate(catch2)
endif()

add_subdirectory("${catch2_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")

########## Targets ##########

add_library(rlbox-cpp17-lib INTERFACE)
target_include_directories(rlbox-cpp17-lib INTERFACE code/include)

########## Tests ##########

add_executable(testing
    code/tests/test_main.cpp
    code/tests/test_conversion.cpp
    code/tests/test_tainted_sizes.cpp
    code/tests/test_tainted_assignment.cpp
    code/tests/test_sandbox_ptr_conversion.cpp
)
target_link_libraries(testing Catch2::Catch2 rlbox-cpp17-lib)

include(CTest)
include(Catch)
catch_discover_tests(testing)

