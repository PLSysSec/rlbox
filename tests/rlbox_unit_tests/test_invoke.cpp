/**
 * @file test_invoke.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that sandbox_invoke works as expected with tainted and simple
 * non-tainted values.
 */

#include "test_include.hpp"

static int test_add_int(int aVal1, int aVal2) { return aVal1 + aVal2; }
static long test_add_long(long aVal1, long aVal2) { return aVal1 + aVal2; }
static float test_add_float(float aVal1, float aVal2) { return aVal1 + aVal2; }
static double test_add_double(double aVal1, double aVal2) {
  return aVal1 + aVal2;
}

TEST_CASE("sandbox_invoke operates correctly with simple tainted int values",
          "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<int> val1 = 5;
  tainted_test<int> val2 = 7;
  tainted_test<int> ret =
      test_sandbox_invoke(sandbox, test_add_int, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}

TEST_CASE("sandbox_invoke operates correctly with simple tainted long values",
          "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<long> val1 = 5;
  tainted_test<long> val2 = 7;
  tainted_test<long> ret =
      test_sandbox_invoke(sandbox, test_add_long, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}

TEST_CASE("sandbox_invoke operates correctly with simple tainted float values",
          "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<float> val1 = 5;
  tainted_test<float> val2 = 7;
  tainted_test<float> ret =
      test_sandbox_invoke(sandbox, test_add_float, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}

TEST_CASE("sandbox_invoke operates correctly with simple tainted double values",
          "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<double> val1 = 5;
  tainted_test<double> val2 = 7;
  tainted_test<double> ret =
      test_sandbox_invoke(sandbox, test_add_double, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}

TEST_CASE(
    "sandbox_invoke operates correctly with simple tainted and unwrapped int "
    "values",
    "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  tainted_test<int> val1 = 5;
  int val2 = 7;
  tainted_test<int> ret =
      test_sandbox_invoke(sandbox, test_add_int, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}

TEST_CASE(
    "sandbox_invoke operates correctly with simple unwrapped and tainted int "
    "values",
    "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();
  int val1 = 5;
  tainted_test<int> val2 = 7;
  tainted_test<int> ret =
      test_sandbox_invoke(sandbox, test_add_int, val1, val2);
  REQUIRE(ret.UNSAFE_unverified() == 12);
  sandbox.destroy_sandbox();
}