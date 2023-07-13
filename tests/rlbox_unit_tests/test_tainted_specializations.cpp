/**
 * @file test_tainted_specializations.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Tainted wrappers are specialized for primitive, pointer, and class
 * types. These wrappers check that the correct specialization is used with
 * static_assert. Check that the various tainted specializations don't generate
 * any pass all specified static_assert.
 */

#include "test_include.hpp"

#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_tainted_relocatable.hpp"

#include "rlbox_noop_sandbox.hpp"

TEST_CASE("Test tainted_fixed_aligned specialization",
          "[rlbox tainted specialization]") {
  [[maybe_unused]] tainted_fixed_aligned<int, rlbox_noop_sandbox> a{};
  [[maybe_unused]] tainted_fixed_aligned<int*, rlbox_noop_sandbox> b{};
  // [[maybe_unused]] tainted_fixed_aligned<int[3], rlbox_noop_sandbox> c{};

  // This is a compile time test so no requires.
}

TEST_CASE("Test tainted_relocatable specialization",
          "[rlbox tainted specialization]") {
  [[maybe_unused]] tainted_relocatable<int, rlbox_noop_sandbox> a{};
  [[maybe_unused]] tainted_relocatable<int*, rlbox_noop_sandbox> b{};
  // [[maybe_unused]] tainted_relocatable<int[3], rlbox_noop_sandbox> c{};

  // This is a compile time test so no requires.
}
