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
  else()
    add_compile_options(-Wall -Wextra -pedantic) # warnings
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

# Clang format ###################

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  # Config in .clang-format
  add_custom_target(format-source
                    COMMAND clang-format
                            -i
                            -style=file
                            ${ALL_CXX_SOURCE_FILES})
endif()


# Dependencies ###################

include(CTest)

if(BUILD_TESTING)
  find_package(Catch2 2.13.7 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(catch2
                         GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                         GIT_TAG v2.13.7)

    FetchContent_GetProperties(catch2)
    if(NOT catch2_POPULATED)
      FetchContent_Populate(catch2)
    endif()

    add_subdirectory("${catch2_SOURCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")
  endif()
endif()

file(GLOB_RECURSE
     RLBOX_SOURCE_FILES
     code/include/*.[chi]pp
     code/include/*.[chi]xx
     code/include/*.cc
     code/include/*.hh
     code/include/*.ii
     code/include/*.[CHI])

# Targets ###################

include(GNUInstallDirs)

find_package(Threads REQUIRED)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/code/include>
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

# Test rlbox features


endif()
