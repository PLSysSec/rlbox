#include <cstdint>
#include <limits>

#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test operator + for numerics", "[operator]")
{
  tainted<int32_t, T_Sbx> a = 3;
  tainted<int32_t, T_Sbx> b = 3 + 4;
  tainted<int32_t, T_Sbx> c = a + 3;
  tainted<int32_t, T_Sbx> d = a + b;
  REQUIRE(a.UNSAFE_Unverified() == 3);
  REQUIRE(b.UNSAFE_Unverified() == 7);
  REQUIRE(c.UNSAFE_Unverified() == 6);
  REQUIRE(d.UNSAFE_Unverified() == 10);

  tainted<uint32_t, T_Sbx> ovWrap = std::numeric_limits<uint32_t>::max();
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

  tainted<uint32_t, T_Sbx> s_a = a;
  tainted<uint32_t, T_Sbx> s_b = b;
  tainted<uint32_t, T_Sbx> s_c = c;
  tainted<uint32_t, T_Sbx> s_d = d;
  tainted<uint32_t, T_Sbx> s_e = e;
  tainted<uint32_t, T_Sbx> s_r = -(((((s_a + s_b) - s_c) * s_d) / s_e));

  REQUIRE(s_r.UNSAFE_Unverified() == r);
}

// NOLINTNEXTLINE
TEST_CASE("Test operator +, - for pointers", "[operator]")
{
  T_Sbx sandbox;
  sandbox.create_sandbox();

  tainted<int32_t*, T_Sbx> pc = sandbox.malloc_in_sandbox<int32_t>();
  tainted<int32_t*, T_Sbx> inc = pc + 1;

  auto diff = reinterpret_cast<char*>(inc.UNSAFE_Unverified()) - // NOLINT
              reinterpret_cast<char*>(pc.UNSAFE_Unverified());   // NOLINT
  REQUIRE(diff == 4);

  tainted<int32_t*, T_Sbx> nullPtr = nullptr;
  REQUIRE_THROWS(nullPtr + 1);

  REQUIRE_THROWS(pc + TestSandbox::SandboxMemorySize);

  tainted<int32_t*, T_Sbx> dec = inc - 1;
  REQUIRE(pc.UNSAFE_Unverified() == dec.UNSAFE_Unverified());

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE(
  "Test operators that produce new values for tainted_volatile numerics",
  "[operator]")
{
  T_Sbx sandbox;
  sandbox.create_sandbox();

  // uint64_t on 64 bit platforms is "unsigned long" which is 64 bits in the app
  // but long is 32-bits in our test sandbox env
  auto pc = sandbox.malloc_in_sandbox<uint64_t>();

  uint64_t max32Val = std::numeric_limits<uint32_t>::max();
  *pc = max32Val;

  const int rhs = 1;
  {
    tainted<uint64_t, T_Sbx> result = (*pc) + rhs;
    uint64_t expected_result = max32Val + rhs;
    REQUIRE(result.UNSAFE_Unverified() == expected_result);
  }

  {
    tainted<uint64_t, T_Sbx> result = (*pc) + (*pc);
    uint64_t expected_result = max32Val + max32Val;
    REQUIRE(result.UNSAFE_Unverified() == expected_result);
  }

  sandbox.destroy_sandbox();
}