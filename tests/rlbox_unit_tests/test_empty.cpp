/**
 * @file test_empty.cpp
 * @brief Check that including the rlbox headers, using
 * RLBOX_DEFINE_BASE_TYPES_FOR, and creating a sbx var doesn't cause issues
 */

#include "catch2/catch.hpp"

#include "rlbox.hpp"  // IWYU pragma: keep

#include "rlbox_noop_sandbox.hpp"  // IWYU pragma: keep

RLBOX_DEFINE_BASE_TYPES_FOR(libtest, rlbox_noop_sandbox);

TEST_CASE("Test include rlbox header", "[rlbox include]") {
  rlbox_sandbox_libtest s;
  (void)s;
  // This is a compile time test so no requires.
}