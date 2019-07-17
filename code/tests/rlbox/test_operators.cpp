#include <cstdint>
#include <limits>

#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test operator + for numerics", "[operator]")
{
  tainted<int32_t, TestSandbox> a = 3;
  tainted<int32_t, TestSandbox> b = 3 + 4;
  tainted<int32_t, TestSandbox> c = a + 3;
  tainted<int32_t, TestSandbox> d = a + b;
  REQUIRE(a.UNSAFE_Unverified() == 3);
  REQUIRE(b.UNSAFE_Unverified() == 7);
  REQUIRE(c.UNSAFE_Unverified() == 6);
  REQUIRE(d.UNSAFE_Unverified() == 10);

  tainted<uint32_t, TestSandbox> ovWrap = std::numeric_limits<uint32_t>::max();
  ovWrap = ovWrap + 1;
  REQUIRE(ovWrap.UNSAFE_Unverified() == 0);
}

TEST_CASE("Test operators that produce new values for numerics", "[operator]")
{
  const uint32_t a = 11;
  const uint32_t b = 17;
  const uint32_t c = 13;
  const uint32_t d = 17;
  const uint32_t e = 2;
  uint32_t r = -(((((a + b) - c) * d) / e));

  tainted<uint32_t, TestSandbox> s_a = a;
  tainted<uint32_t, TestSandbox> s_b = b;
  tainted<uint32_t, TestSandbox> s_c = c;
  tainted<uint32_t, TestSandbox> s_d = d;
  tainted<uint32_t, TestSandbox> s_e = e;
  tainted<uint32_t, TestSandbox> s_r = -(((((s_a + s_b) - s_c) * s_d) / s_e));

  REQUIRE(s_r.UNSAFE_Unverified() == r);
}

// NOLINTNEXTLINE
TEST_CASE("Test operator +, - for pointers", "[operator]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  tainted<int32_t*, TestSandbox> pc = sandbox.malloc_in_sandbox<int32_t>();
  tainted<int32_t*, TestSandbox> inc = pc + 1;

  auto diff = reinterpret_cast<char*>(inc.UNSAFE_Unverified()) - // NOLINT
              reinterpret_cast<char*>(pc.UNSAFE_Unverified());   // NOLINT
  REQUIRE(diff == 4);

  tainted<int32_t*, TestSandbox> nullPtr = nullptr;
  // operation on null pointer should throw
  REQUIRE_THROWS(nullPtr + 1);

  // pointer addition overflow sandbox bounds should throw
  REQUIRE_THROWS(pc + TestSandbox::SandboxMemorySize);

  tainted<int32_t*, TestSandbox> dec = inc - 1;
  REQUIRE(pc.UNSAFE_Unverified() == dec.UNSAFE_Unverified());

  auto pc2 = sandbox.malloc_in_sandbox<char>();
  auto inc2 = pc2 + 1;

  auto diff2 = reinterpret_cast<char*>(inc2.UNSAFE_Unverified()) - // NOLINT
               reinterpret_cast<char*>(pc2.UNSAFE_Unverified());   // NOLINT
  REQUIRE(diff2 == 1);

  auto pc3 = sandbox.malloc_in_sandbox<int32_t*>();
  auto inc3 = pc3 + 1;

  auto diff3 = reinterpret_cast<char*>(inc3.UNSAFE_Unverified()) - // NOLINT
               reinterpret_cast<char*>(pc3.UNSAFE_Unverified());   // NOLINT
  REQUIRE(diff3 == sizeof(TestSandbox::T_PointerType));

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE(
  "Test operators that produce new values for tainted_volatile numerics",
  "[operator]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  // uint64_t on 64 bit platforms is "unsigned long" which is 64 bits in the app
  // but long is 32-bits in our test sandbox env
  auto pc = sandbox.malloc_in_sandbox<uint64_t>();

  uint64_t max32Val = std::numeric_limits<uint32_t>::max();
  *pc = max32Val;

  const int rhs = 1;
  {
    tainted<uint64_t, TestSandbox> result = (*pc) + rhs;
    uint64_t expected_result = max32Val + rhs;
    REQUIRE(result.UNSAFE_Unverified() == expected_result);
  }

  {
    tainted<uint64_t, TestSandbox> result = (*pc) + (*pc);
    uint64_t expected_result = max32Val + max32Val;
    REQUIRE(result.UNSAFE_Unverified() == expected_result);
  }

  sandbox.destroy_sandbox();
}