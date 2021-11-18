#include <cstdint>
#include <memory>
#include <utility>

#include "test_include.hpp"

using rlbox::tainted;
using rlbox::tainted_boolean_hint;
using rlbox::tainted_int_hint;

// NOLINTNEXTLINE
TEST_CASE("RLBox test basic verification", "[verification]")
{
  const auto testVal = 5;
  const auto lb = 0;
  const auto ub = 10;

  tainted<int, TestSandbox> test = testVal;
  auto result = test.copy_and_verify(
    [&](int val) { return val > lb && val < ub ? val : -1; });
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

  tainted<Example_Enum, TestSandbox> ref = ENUM_FIRST;
  auto enumVal = ref.copy_and_verify(
    [](Example_Enum val) { return val <= ENUM_THIRD ? val : ENUM_UNKNOWN; });
  UNUSED(enumVal);
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test pointer verification", "[verification]")
{
  const auto testVal = 5;
  const auto lb = 0;
  const auto ub = 10;

  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  tainted<int*, TestSandbox> pa = sandbox.malloc_in_sandbox<int>();
  *pa = testVal;

  auto result1 = pa.copy_and_verify([&](std::unique_ptr<int> val) {
    return *val > lb && *val < ub ? std::move(val) : nullptr;
  });
  REQUIRE(result1 != nullptr);
  REQUIRE(*result1 == testVal);

  auto result2 = pa.copy_and_verify_address([](uintptr_t val) { return val; });
  REQUIRE(pa.UNSAFE_unverified() == reinterpret_cast<int*>(result2)); // NOLINT

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test function pointer verification", "[verification]")
{
  using T_Func = int (*)(int);

  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  tainted<T_Func, TestSandbox> a = nullptr;
  REQUIRE(a.UNSAFE_unverified() == nullptr);
  REQUIRE_COMPILE_ERR(
    a.copy_and_verify([](std::unique_ptr<T_Func> val) { return val; }));
  REQUIRE(a.copy_and_verify_address([](uintptr_t val) {
    return reinterpret_cast<void*>(val); // NOLINT
  }) == nullptr);

  // Disabled until function pointers are handled correctly
  // auto b = sandbox.malloc_in_sandbox<T_Func>();
  // *b = a;

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("RLBox tainted hint verification", "[verification]")
{
  tainted_boolean_hint a = true;
  REQUIRE_COMPILE_ERR(a.copy_and_verify([](bool val) { return val; }));

  tainted_int_hint b = 1;
  REQUIRE_COMPILE_ERR(b.copy_and_verify());
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test unverified value", "[verification]")
{
  const auto testVal = 5;
  tainted<int, TestSandbox> test = testVal;
  auto result1 = test.UNSAFE_unverified();
  auto result2 = test.copy_and_verify([](int val) { return val; });
  auto result3 = test.unverified_safe_because("Reason: testing");
  REQUIRE(result1 == testVal);
  REQUIRE(result2 == testVal);
  REQUIRE(result3 == testVal);
}

// NOLINTNEXTLINE
TEST_CASE("RLBox test unverified pointer", "[verification]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();
  tainted<int*, TestSandbox> pa = sandbox.malloc_in_sandbox<int>();

  const auto elementCountSafe = 1;
  REQUIRE_NOTHROW(pa.unverified_safe_pointer_because(elementCountSafe,
                                                     "Reading within range"));

  const auto elementCountUnsafe =
    (TestSandbox::SandboxMemorySize / sizeof(int)) + 1;
  REQUIRE_THROWS(pa.unverified_safe_pointer_because(
    elementCountUnsafe, "Definitely out of sandbox memory. Unsafe!"));

  sandbox.destroy_sandbox();
}
