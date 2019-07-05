#include "test_include.hpp"

using rlbox::RLBox_Verify_Status;
using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("RLBox test basic verification", "[verification]")
{
  const auto testVal = 5;
  const auto lb = 0;
  const auto ub = 10;

  tainted<int, T_Sbx> test = testVal;
  auto result = test.copy_and_verify(
    [](int val) {
      return val > lb && val < ub ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
    },
    -1);
  REQUIRE(result == 5);
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test enum verification", "[verification]")
{
  using Example_Enum = enum {
    ENUM_UNKNOWN,
    ENUM_FIRST,
    ENUM_SECOND,
    ENUM_THIRD
  };

  tainted<Example_Enum, T_Sbx> ref = ENUM_FIRST;
  auto enumVal = ref.copy_and_verify(
    [](Example_Enum val) {
      return val <= ENUM_THIRD ? RLBox_Verify_Status::SAFE
                               : RLBox_Verify_Status::UNSAFE;
    },
    ENUM_UNKNOWN);
  UNUSED(enumVal);
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test pointer verification", "[verification]")
{
  const auto testVal = 5;
  const auto lb = 0;
  const auto ub = 10;
  const auto def = -1;

  T_Sbx sandbox;
  sandbox.create_sandbox();

  tainted<int*, T_Sbx> pa = sandbox.malloc_in_sandbox<int>();
  *pa = testVal;

  auto result1 = pa.copy_and_verify(
    [](const int* val) {
      return *val > lb && *val < ub ? RLBox_Verify_Status::SAFE
                                    : RLBox_Verify_Status::UNSAFE;
    },
    def);
  REQUIRE(result1 == testVal);

  sandbox.destroy_sandbox();
}
