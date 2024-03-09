cmake_minimum_required(VERSION 3.13)

project(rlbox-sandboxing-api
        VERSION 0.9
        DESCRIPTION "RLBox sandboxing API"
        HOMEPAGE_URL "https://github.com/PLSysSec/rlbox")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Project Settings ###################

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  # ignore warnings about macro invocations with no parameters
  add_compile_options(/wd4002)
  # ignore warnings about using unsafe strcpy vs strcpy_s define _CRT_SECURE_NO_WARNINGS.
  add_compile_options(/D _CRT_SECURE_NO_WARNINGS)
  # Make dll's export all symbols, similar to unix ELF .so filess
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

option(DEV "Use settings suitable for dev contributions to rlbox (Off by default)" OFF)


file(GLOB_RECURSE
     RLBOX_SOURCE_FILES
     include/*.hpp)

file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     include/*.hpp
     tests/*.hpp
     tests/*.cpp)

file(GLOB_RECURSE
     RLBOX_UNIT_TESTS_FILES
     tests/rlbox_unit_tests/*.cpp)

# Dependencies ###################

include(CTest)

if(BUILD_TESTING)
  find_package(Catch2 2.13.9 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(catch2
                         GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                         GIT_TAG v2.13.9)

    FetchContent_GetProperties(catch2)
    if(NOT catch2_POPULATED)
      FetchContent_Populate(catch2)
    endif()

    add_subdirectory("${catch2_SOURCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")
  endif()
endif()

if(DEV)
  find_program(IWYU "iwyu")
  if(IWYU)
    include(FetchContent)
    FetchContent_Declare(iwyu_wrapper
                         GIT_REPOSITORY https://github.com/shravanrn/iwyu_wrapper.git)

    FetchContent_GetProperties(iwyu_wrapper)
    if(NOT iwyu_wrapper_POPULATED)
      FetchContent_Populate(iwyu_wrapper)
    endif()
  endif()
endif()

# Dev Tools ###################

if(DEV)
  if(MSVC)
    message("Enabling warnings")
    add_compile_options(/W4) # warnings
  else()
    message("Enabling sanitizers and warnings")
    add_compile_options(-Wall -Wextra -pedantic) # warnings
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments) # Disable token pasting warning
    # add_compile_options(-fsanitize=address)
    # add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  endif()

  find_program(CLANG_FORMAT "clang-format")
  if(CLANG_FORMAT)
    message("Enabling ${CLANG_FORMAT} check")
    # Config in .clang-format
    # When runs, this actually formats the files
    add_custom_target(format-source
      COMMAND clang-format -i -style=file ${ALL_CXX_SOURCE_FILES})
  endif()

  find_program(CLANG_TIDY "clang-tidy")
  if(CLANG_TIDY)
    message("Enabling ${CLANG_TIDY} check")
    # Config in .clang-tidy
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
  endif()

  find_program(IWYU "iwyu")
  if(IWYU)
    message("Enabling ${IWYU} check")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
        python3 ${iwyu_wrapper_SOURCE_DIR}/iwyu_wrapper.py
        -Xiwyu --mapping_file="${CMAKE_SOURCE_DIR}/iwyu.imp"
        -Xiwyu --transitive_includes_only
        -Xiwyu --no_fwd_decls
        --no-error)

    # We need to manually check .hpp files
    # See https://github.com/include-what-you-use/include-what-you-use/issues/633
    #
    # The suggested fix of --check_also does not work correctly
    #
    # iwyu doesn't show include file issues unless invoked on the hpp itself
    # But, CMake only invokes it on source files.
    #
    # Calling iwyu directly is difficult since until very recently, it results
    # in non-zero error codes on success. Call our wrapper that helps with this

    add_custom_target(iwyu-check ALL
      DEPENDS ${RLBOX_SOURCE_FILES}
      COMMAND python3 ${iwyu_wrapper_SOURCE_DIR}/iwyu_wrapper.py
        -Xiwyu --mapping_file="${CMAKE_SOURCE_DIR}/iwyu.imp"
        -Xiwyu --transitive_includes_only
        -Xiwyu --no_fwd_decls
        --no-error
        # Add arguments that CMake would normally add
        -std=c++17
        -I ${CMAKE_SOURCE_DIR}/include
        -- ${RLBOX_SOURCE_FILES}
      COMMENT "Running iwyu on rlbox source"
    )

  endif()

  if(CLANG_FORMAT)
    # Config in .clang-format
    # Just check if there are changes, don't make any changes
    add_custom_target(check-format-source ALL
      COMMAND clang-format -style=file --dry-run --ferror-limit=5 --Werror ${ALL_CXX_SOURCE_FILES})
  endif()

 find_program(CPPCHECK "cppcheck")
 if(CPPCHECK)
   message("Enabling ${CPPCHECK} check")
   # Just check the rlbox header files
   # Note we configure cppcheck to assume no custom abort handler
   add_custom_target(cppcheck ALL
     COMMAND cppcheck
       --enable=warning,performance,portability,information,missingInclude -URLBOX_CUSTOM_ABORT
       --suppress=missingIncludeSystem
       --std=c++17 -I ${CMAKE_SOURCE_DIR}/include --error-exitcode=1 --quiet
       ${RLBOX_SOURCE_FILES})
 endif()

endif()

# Documentation ###################

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
  message("Found Doxygen")

  # Technique from https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/
  set(DOXYGEN_PROJECT_NAME "RLBox")
  set(DOXYGEN_INPUT_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
  set(DOXYGEN_INPUT_UNIT_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/rlbox_unit_tests)
  set(DOXYGEN_INPUT_PLUGIN_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/rlbox_plugin_tests)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # Replace variables inside @@ with the current values
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                     DEPENDS ${RLBOX_SOURCE_FILES} ${DOXYFILE_OUT}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                     MAIN_DEPENDENCY ${DOXYFILE_OUT}
                     ${DOXYFILE_IN}
                     COMMENT "Generating Doxygen documentation")

  add_custom_target(docs DEPENDS ${DOXYGEN_INDEX_FILE})

endif()

# Targets ###################

include(GNUInstallDirs)

find_package(Threads REQUIRED)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${RLBOX_SOURCE_FILES}")

# Install ###################

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${CMAKE_PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

set(DATAROOT_CONFIG_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME}")
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${DATAROOT_CONFIG_DESTINATION}
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR)
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rlbox"
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${DATAROOT_CONFIG_DESTINATION})
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${DATAROOT_CONFIG_DESTINATION})

# Tests ###################

if(BUILD_TESTING)

  include(Catch)

  #### Test rlbox features

  add_executable(rlbox_unit_tests
                 tests/test_main.cpp
                 ${RLBOX_UNIT_TESTS_FILES})

  target_compile_definitions(rlbox_unit_tests PUBLIC
    RLBOX_USE_EXCEPTIONS_ON_ERROR
    RLBOX_REPLACE_COMPILE_CHECKS_WITH_RUNTIME_ERRORS)

  target_link_libraries(rlbox_unit_tests Catch2::Catch2 ${PROJECT_NAME})

  catch_discover_tests(rlbox_unit_tests)

  #### Test rlbox_noop_sandbox features

  add_executable(rlbox_noop_sandbox_plugin_tests
                 tests/test_main.cpp
                 tests/test_rlbox_noop_sandbox.cpp)

  target_compile_definitions(rlbox_unit_tests PUBLIC
    RLBOX_USE_EXCEPTIONS_ON_ERROR
    RLBOX_REPLACE_COMPILE_CHECKS_WITH_RUNTIME_ERRORS)

  target_link_libraries(rlbox_noop_sandbox_plugin_tests Catch2::Catch2 ${PROJECT_NAME})

  catch_discover_tests(rlbox_noop_sandbox_plugin_tests)

  #### Add check command to run tests

  add_custom_target(
    check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND
      ${CMAKE_COMMAND} -E env
      LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/leak_suppressions.txt
      UBSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/ub_suppressions.txt
      ${CMAKE_CTEST_COMMAND} -V
    DEPENDS rlbox_unit_tests rlbox_noop_sandbox_plugin_tests
    COMMENT "Running tests"
  )

endif()
