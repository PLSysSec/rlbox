#pragma once

/**
 * @file test_include.hpp
 * @brief Include file to be used by tests. Contains some common headers and
 * functions used by tests
 */

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

#include "rlbox.hpp"
// IWYU pragma: end_exports

#include <iostream>

using namespace rlbox;

/**
 * @brief Helper type that prints stringified version of the types given to it
 *
 * @tparam T is the set of types to print
 */
template <typename... T>
void rlbox_test_helper_print_type() {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  std::cout << "Func: " << __FUNCSIG__ << std::endl;
#else
  std::cout << "Func: " << __PRETTY_FUNCTION__ << std::endl;
#endif
}