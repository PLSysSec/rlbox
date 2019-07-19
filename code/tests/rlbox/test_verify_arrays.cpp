#include <array>
#include <cstdint>
#include <cstring>

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

  auto result_fail = pa->copy_and_verify(
    [](std::array<long, 4>) { // NOLINT
      return RLBox_Verify_Status::UNSAFE;
    },
    def);

  REQUIRE(result_fail[0] == defaultVal1);
  REQUIRE(result_fail[1] == defaultVal2);
  REQUIRE(result_fail[2] == defaultVal3);
  REQUIRE(result_fail[3] == defaultVal4);

  auto result_success = pa->copy_and_verify(
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

// NOLINTNEXTLINE
TEST_CASE("RLBox test range verification", "[verification]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  // long long is the 64 bit type in the TestSandbox
  const unsigned long long val64 = 0x1234567890ABCDEF;       // NOLINT
  auto pa = sandbox.malloc_in_sandbox<unsigned long long>(); // NOLINT
  *pa = val64;

  // int is the 32 bit type
  rlbox::tainted<unsigned int*, TestSandbox> pa_cast =
    rlbox::sandbox_reinterpret_cast<unsigned int*>(pa); // NOLINT

  auto checked_range = pa_cast.copy_and_verify_range(
    [](unsigned int*) { // NOLINT
      return RLBox_Verify_Status::SAFE;
    },
    2,
    nullptr);

  auto val32_ptr = reinterpret_cast<const uint32_t*>(&val64); // NOLINT
  REQUIRE(checked_range[0] == val32_ptr[0]);                  // NOLINT
  REQUIRE(checked_range[1] == val32_ptr[1]);                  // NOLINT
  REQUIRE(sandbox.is_pointer_in_app_memory(checked_range));

  delete[] checked_range; // NOLINT

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test string verification", "[verification]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const uint32_t max_length = 100;
  auto pc = sandbox.malloc_in_sandbox<char>(max_length); // NOLINT

  std::strncpy(pc.UNSAFE_Unverified(), "Hello", max_length);

  auto checked_string = pc.copy_and_verify_string(
    [](char*) { // NOLINT
      return RLBox_Verify_Status::SAFE;
    },
    nullptr);

  REQUIRE(strcmp(checked_string, "Hello") == 0); // NOLINT
  REQUIRE(sandbox.is_pointer_in_app_memory(checked_string));

  delete[] checked_string; // NOLINT

  sandbox.destroy_sandbox();
}