#include <cstdint>
#include <limits>
#include <type_traits>

#include "test_include.hpp"

using rlbox::tainted;
using rlbox::tainted_volatile;

// NOLINTNEXTLINE
TEST_CASE("tainted assignment operates correctly", "[tainted_assignment]")
{
  const int RandomVal1 = 4;
  const int RandomVal2 = 5;

  // Avoid warnings about uninitialized var
  tainted<int, TestSandbox> a; // NOLINT
  a = RandomVal1;
  tainted<int, TestSandbox> b = RandomVal2;
  tainted<int, TestSandbox> c = b;
  tainted<int, TestSandbox> d; // NOLINT
  d = b;
  const float FloatVal1 = 2.4;
  tainted<float, TestSandbox> e = 2.4;          // NOLINT
  REQUIRE(a.UNSAFE_unverified() == RandomVal1); // NOLINT
  REQUIRE(b.UNSAFE_unverified() == RandomVal2); // NOLINT
  REQUIRE(c.UNSAFE_unverified() == RandomVal2); // NOLINT
  REQUIRE(d.UNSAFE_unverified() == RandomVal2); // NOLINT
  REQUIRE(e.UNSAFE_unverified() == FloatVal1);  // NOLINT
}

// NOLINTNEXTLINE
TEST_CASE("tainted_volatile assignment operates correctly",
          "[tainted_assignment]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  // On 64 bit platforms, "unsigned long" is 64 bits in the app
  // but unsigned long is 32-bits in our test sandbox env
  // NOLINTNEXTLINE(google-runtime-int)
  auto pc = sandbox.malloc_in_sandbox<unsigned long>();

  // Only run this test for platforms where unsigned long is 64 bits
  // NOLINTNEXTLINE(google-runtime-int)
  if constexpr (sizeof(unsigned long) == sizeof(uint64_t)) {
    uint64_t max32Val = std::numeric_limits<uint32_t>::max();
    *pc = max32Val;

    REQUIRE((*pc).UNSAFE_unverified() == max32Val);
    REQUIRE(pc->UNSAFE_unverified() == max32Val);

    uint64_t max64Val = std::numeric_limits<uint64_t>::max();
    REQUIRE_THROWS(*pc = max64Val);
  }

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted_assignment]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
  REQUIRE(std::is_same_v<decltype(ptr), tainted<uint32_t*, TestSandbox>>);
  REQUIRE(ptr.UNSAFE_unverified() != nullptr);

  auto& val = *ptr;
  REQUIRE(
    std::is_same_v<decltype(val), tainted_volatile<uint32_t, TestSandbox>&>);
  REQUIRE(
    std::is_same_v<decltype(tainted(val)), tainted<uint32_t, TestSandbox>>);

  REQUIRE(
    std::is_same_v<decltype(tainted(&val)), tainted<uint32_t*, TestSandbox>>);
  REQUIRE(
    std::is_same_v<decltype(tainted(&*ptr)), tainted<uint32_t*, TestSandbox>>);

  tainted<uint32_t**, TestSandbox> ptr2 =
    sandbox.malloc_in_sandbox<uint32_t*>();
  auto& deref = *ptr2;
  REQUIRE(
    std::is_same_v<decltype(deref), tainted_volatile<uint32_t*, TestSandbox>&>);
  REQUIRE(
    std::is_same_v<decltype(*deref), tainted_volatile<uint32_t, TestSandbox>&>);

  REQUIRE(
    std::is_same_v<decltype(**ptr2), tainted_volatile<uint32_t, TestSandbox>&>);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("tainted pointer assignments", "[tainted_assignment]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  tainted<int**, TestSandbox> pa = nullptr;
  pa = nullptr;
  tainted<int**, TestSandbox> pb = nullptr;
  pb = nullptr;

  tainted<int***, TestSandbox> pc = sandbox.malloc_in_sandbox<int**>();
  *pc = nullptr;
  pb = *pc;

  tainted<void**, TestSandbox> pv = sandbox.malloc_in_sandbox<void*>();
  *pv = nullptr;

  sandbox.destroy_sandbox();
}