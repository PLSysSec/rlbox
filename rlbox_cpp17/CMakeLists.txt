cmake_minimum_required(VERSION 3.11)

project(rlbox-cpp17 VERSION 0.1 DESCRIPTION "RLBox safe sandboxing API in C++17")

add_library(rlbox-cpp17-lib INTERFACE)
target_include_directories(rlbox-cpp17-lib INTERFACE include)
target_compile_features(rlbox-cpp17-lib INTERFACE cxx_std_17)

######## Dependencies ########
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

########## Dev Tools ##########

# Configs in .clang-tidy
set(CMAKE_CXX_CLANG_TIDY clang-tidy;)


##############################

add_executable(rlbox-cpp17-tests
    tests/test_main.cpp
    tests/test_convert_fundamental.cpp
)
target_compile_features(rlbox-cpp17-tests INTERFACE cxx_std_17)
target_link_libraries(rlbox-cpp17-tests Catch2::Catch2 rlbox-cpp17-lib)
