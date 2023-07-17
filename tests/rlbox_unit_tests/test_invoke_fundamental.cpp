/**
 * @file test_invoke_fundamental.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that sandbox_invoke works as expected with tainted and simple
 * non-tainted values.
 */

#include "test_include.hpp"

#include <limits.h>

static int test_add_int(int aVal1, int aVal2) { return aVal1 + aVal2; }
static long test_add_long(long aVal1, long aVal2) { return aVal1 + aVal2; }
static float test_add_float(float aVal1, float aVal2) { return aVal1 + aVal2; }
static double test_add_double(double aVal1, double aVal2) {
  return aVal1 + aVal2;
}
static uint16_t test_add_uint16_t(uint16_t aVal1, uint16_t aVal2) {
  return aVal1 + aVal2;
}
static uint32_t test_add_uint32_t(uint32_t aVal1, uint32_t aVal2) {
  return aVal1 + aVal2;
}
static uint64_t test_add_uint64_t(uint64_t aVal1, uint64_t aVal2) {
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

TEST_CASE("sandbox_invoke operates correctly with u16s", "[sandbox_invoke]") {
  rlbox_sandbox_test_smallerabi sandbox;
  sandbox.create_sandbox();
  const uint16_t val1 = std::numeric_limits<uint16_t>::max();
  const uint16_t val2 = 5;
  const auto expected = static_cast<uint16_t>(val1 + val2);
  tainted_test_smallerabi<rlbox_uint16_t> t_val1 = val1;
  tainted_test_smallerabi<rlbox_uint16_t> t_val2 = val2;
  tainted_test_smallerabi<rlbox_uint16_t> ret = test_smallerabi_sandbox_invoke(
      sandbox, test_add_uint16_t, t_val1, t_val2);
  REQUIRE(ret.UNSAFE_unverified() == expected);
  sandbox.destroy_sandbox();
}

TEST_CASE("sandbox_invoke operates correctly with u32s", "[sandbox_invoke]") {
  rlbox_sandbox_test_smallerabi sandbox;
  sandbox.create_sandbox();
  const uint32_t val1 = std::numeric_limits<uint32_t>::max();
  const uint32_t val2 = 5;
  const auto expected = static_cast<uint32_t>(val1 + val2);
  tainted_test_smallerabi<rlbox_uint32_t> t_val1 = val1;
  tainted_test_smallerabi<rlbox_uint32_t> t_val2 = val2;
  tainted_test_smallerabi<rlbox_uint32_t> ret = test_smallerabi_sandbox_invoke(
      sandbox, test_add_uint32_t, t_val1, t_val2);
  REQUIRE(ret.UNSAFE_unverified() == expected);
  sandbox.destroy_sandbox();
}

TEST_CASE("sandbox_invoke operates correctly with u64s", "[sandbox_invoke]") {
  rlbox_sandbox_test_smallerabi sandbox;
  sandbox.create_sandbox();
  const uint64_t val1 = std::numeric_limits<uint64_t>::max();
  const uint64_t val2 = 5;
  const auto expected = static_cast<uint64_t>(val1 + val2);
  tainted_test_smallerabi<rlbox_uint64_t> t_val1 = val1;
  tainted_test_smallerabi<rlbox_uint64_t> t_val2 = val2;
  tainted_test_smallerabi<rlbox_uint64_t> ret = test_smallerabi_sandbox_invoke(
      sandbox, test_add_uint64_t, t_val1, t_val2);
  REQUIRE(ret.UNSAFE_unverified() == expected);
  sandbox.destroy_sandbox();
}
