#include <array>
#include <cstdint>

#include "test_include.hpp"

using rlbox::RLBox_Verify_Status;

// NOLINTNEXTLINE
TEST_CASE("RLBox test array verification", "[verification]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto pa = sandbox.malloc_in_sandbox<long[4]>(); // NOLINT

  const int32_t testVal1 = 10;
  const int32_t testVal2 = 11;
  const int32_t testVal3 = 12;
  const int32_t testVal4 = 13;

  (*pa)[0] = testVal1;
  (*pa)[1] = testVal2;
  (*pa)[2] = testVal3;
  (*pa)[3] = testVal4;

  const int32_t defaultVal1 = 210;
  const int32_t defaultVal2 = 211;
  const int32_t defaultVal3 = 212;
  const int32_t defaultVal4 = 213;
  std::array<long, 4> def = // NOLINT
    { defaultVal1, defaultVal2, defaultVal3, defaultVal4 };

  auto result_fail = pa->copy_and_verify_array(
    [](std::array<long, 4>) { // NOLINT
      return RLBox_Verify_Status::UNSAFE;
    },
    def);

  REQUIRE(result_fail[0] == defaultVal1);
  REQUIRE(result_fail[1] == defaultVal2);
  REQUIRE(result_fail[2] == defaultVal3);
  REQUIRE(result_fail[3] == defaultVal4);

  auto result_success = pa->copy_and_verify_array(
    [](std::array<long, 4>) { // NOLINT
      return RLBox_Verify_Status::SAFE;
    },
    def);

  REQUIRE(result_success[0] == testVal1);
  REQUIRE(result_success[1] == testVal2);
  REQUIRE(result_success[2] == testVal3);
  REQUIRE(result_success[3] == testVal4);

  sandbox.destroy_sandbox();
}
