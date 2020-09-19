cmake_minimum_required(VERSION 3.13)

project(rlbox-cpp17
        VERSION 0.1
        DESCRIPTION "RLBox safe sandboxing API in C++17")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Project Settings ###################

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DEV "Use settings suitable for dev contributions to rlbox" OFF)

file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     code/*.[chi]pp
     code/*.[chi]xx
     code/*.cc
     code/*.hh
     code/*.ii
     code/*.[CHI])

# Dev Tools ###################

if(DEV)
  if(MSVC)
    add_compile_options(/W4) # warnings
    add_compile_options(/WX) # warnings as errors
  else()
    add_compile_options(-Wall -Wextra -pedantic) # warnings
    add_compile_options(-Werror) # warnings as errors
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  endif()

  find_program(CLANG_TIDY "clang-tidy")
  if(CLANG_TIDY)
    # Config in .clang-tidy
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
  endif()

  find_program(CLANG_FORMAT "clang-format")
  if(CLANG_FORMAT)
    # Config in .clang-format
    add_custom_target(format-source
                      COMMAND clang-format
                              -i
                              -style=file
                              ${ALL_CXX_SOURCE_FILES})
  endif()

  find_program(IWYU "iwyu")
  if("")#IWYU)# TODO: Still have to ensure this checks .hpp files, which it
         # doesn't right now. See bug below about "check_also"... which doesn't
         # seem to work https://github.com/include-what-you-use/include-what-
         # you-use/issues/633
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
        iwyu
        -Xiwyu
        --transitive_includes_only
        -Xiwyu
        --check_also="*.hpp"
        # -Xiwyu
        # --verbose=4
        -Xiwyu --mapping_file=${CMAKE_SOURCE_DIR}/iwyu.imp)
  endif()

endif()

# Dependencies ###################

include(FetchContent)
FetchContent_Declare(catch2
                     GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                     GIT_TAG v2.9.1)

FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
  FetchContent_Populate(catch2)
endif()

add_subdirectory("${catch2_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")

# Documentation ###################
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
  # Technique from https://devblogs.microsoft.com/cppblog/clear-functional-c
  # -documentation-with-sphinx-breathe-doxygen-cmake/
  set(DOXYGEN_PROJECT_NAME "RLBox")
  set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/code/include)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # Replace variables inside @@ with the current values
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  file(GLOB_RECURSE
       RLBOX_SOURCE_FILES
       code/include/*.[chi]pp
       code/include/*.[chi]xx
       code/include/*.cc
       code/include/*.hh
       code/include/*.ii
       code/include/*.[CHI])

  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                     DEPENDS ${RLBOX_SOURCE_FILES}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                     MAIN_DEPENDENCY ${DOXYFILE_OUT}
                     ${DOXYFILE_IN}
                     COMMENT "Generating Doxygen documentation")

  add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

  find_package(Sphinx QUIET)
  if(SPHINX_FOUND)
    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs-src)
    set(SPHINX_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/docs/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

    # Only regenerate Sphinx when: - Doxygen has rerun - Our doc files have been
    # updated - The Sphinx config has been updated
    add_custom_command(
      OUTPUT ${SPHINX_INDEX_FILE}
      COMMAND ${SPHINX_EXECUTABLE}
              -b
              html
              -Dbreathe_projects.RLBox=${DOXYGEN_OUTPUT_DIR}/xml
              ${SPHINX_SOURCE}
              ${SPHINX_BUILD}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${SPHINX_SOURCE}/index.rst ${DOXYGEN_INDEX_FILE}
      MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
      COMMENT "Generating Sphinx documentation")

    add_custom_target(docs ALL DEPENDS ${SPHINX_INDEX_FILE})
  endif()

endif()

# Targets ###################

find_package(Threads REQUIRED)
add_library(rlbox-cpp17-lib INTERFACE)
target_include_directories(rlbox-cpp17-lib INTERFACE code/include)
target_link_libraries(rlbox-cpp17-lib INTERFACE ${CMAKE_THREAD_LIBS_INIT})

# Tests ###################

include(CTest)
include(Catch)

# Test rlbox features
add_executable(test_rlbox
               code/tests/test_main.cpp
               code/tests/rlbox/test_bool_operators.cpp
               code/tests/rlbox/test_callback.cpp
               code/tests/rlbox/test_comparison.cpp
               code/tests/rlbox/test_conversion.cpp
               code/tests/rlbox/test_operators.cpp
               code/tests/rlbox/test_sandbox_function_assignment.cpp
               code/tests/rlbox/test_sandbox_noop_sandbox.cpp
               code/tests/rlbox/test_sandbox_noop_sandbox_invoke_fail.cpp
               code/tests/rlbox/test_sandbox_ptr_conversion.cpp
               code/tests/rlbox/test_stdlib.cpp
               code/tests/rlbox/test_tainted_assignment.cpp
               code/tests/rlbox/test_tainted_opaque.cpp
               code/tests/rlbox/test_tainted_sizes.cpp
               code/tests/rlbox/test_tainted_structs.cpp
               code/tests/rlbox/test_type_traits.cpp
               code/tests/rlbox/test_verification.cpp
               code/tests/rlbox/test_verify_arrays.cpp
               code/tests/rlbox/test_wrapper_traits.cpp)

target_include_directories(test_rlbox PRIVATE code/tests/rlbox)

target_link_libraries(test_rlbox Catch2::Catch2 rlbox-cpp17-lib)

catch_discover_tests(test_rlbox)

# Test rlbox overheads

add_executable(test_rlbox_transition_timers
               code/tests/test_main.cpp
               code/tests/rlbox/test_sandbox_transition_timings.cpp)

target_include_directories(test_rlbox_transition_timers PRIVATE code/tests/rlbox)

target_link_libraries(test_rlbox_transition_timers Catch2::Catch2 rlbox-cpp17-lib)

catch_discover_tests(test_rlbox_transition_timers)

# Test rlbox transition customization

add_executable(test_rlbox_transition_customization
               code/tests/test_main.cpp
               code/tests/rlbox/test_sandbox_transition_customization.cpp)

target_include_directories(test_rlbox_transition_customization PRIVATE code/tests/rlbox)

target_link_libraries(test_rlbox_transition_customization Catch2::Catch2 rlbox-cpp17-lib)

catch_discover_tests(test_rlbox_transition_customization)

# Test rlbox glue
add_library(rlbox_glue_lib_static STATIC code/tests/rlbox_glue/lib/libtest.c)
target_include_directories(rlbox_glue_lib_static
                           PUBLIC code/tests/rlbox_glue/lib)

add_library(rlbox_glue_lib_shared SHARED code/tests/rlbox_glue/lib/libtest.c)
target_include_directories(rlbox_glue_lib_shared
                           PUBLIC code/tests/rlbox_glue/lib)

add_executable(test_rlbox_glue
               code/tests/test_main.cpp
               code/tests/rlbox_glue/test_noop_sandbox_glue.cpp
               code/tests/rlbox_glue/test_noop_sandbox_glue_embedder_vars.cpp)
target_include_directories(test_rlbox_glue PUBLIC code/tests/rlbox_glue)

target_link_libraries(test_rlbox_glue
                      Catch2::Catch2
                      rlbox-cpp17-lib
                      rlbox_glue_lib_static)

catch_discover_tests(test_rlbox_glue)

# Shortcuts ###################

add_custom_target(
  check
  COMMAND
    LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/leak_suppressions.txt
    UBSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/ub_suppressions.txt
    ${CMAKE_CTEST_COMMAND} -V)
add_dependencies(check test_rlbox test_rlbox_transition_timers test_rlbox_glue)
