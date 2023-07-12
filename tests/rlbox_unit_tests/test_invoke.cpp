/**
 * @file test_invoke.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that sandbox_invoke works as expected with tainted and simple
 * non-tainted values.
 */

#include "test_include.hpp"

static int test_add(int aVal1, int aVal2) { return aVal1 + aVal2; }

TEST_CASE("sandbox_invoke operates correctly with simple tainted values",
          "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<int> val1 = 5;
  tainted_test<int> val2 = 7;
  tainted_test<int> ret = sandbox_invoke(sandbox, test_add, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}
