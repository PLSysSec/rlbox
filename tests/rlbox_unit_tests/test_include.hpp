#pragma once

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

#include "rlbox.hpp"
// IWYU pragma: end_exports

#include <iostream>

template<typename... T>
void rlbox_test_helper_print_type()
{
  std::cout << "Func: " << __PRETTY_FUNCTION__;
}