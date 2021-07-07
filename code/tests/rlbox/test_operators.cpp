#include <cstdint>
#include <limits>
#include <memory>

#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test operator + for numerics", "[operator]")
{
  tainted<int32_t, TestSandbox> a = 3;
  tainted<int32_t, TestSandbox> b = 3 + 4;
  tainted<int32_t, TestSandbox> c = a + 3;
  tainted<int32_t, TestSandbox> d = 3 + a;
  tainted<int32_t, TestSandbox> e = a + b;
  REQUIRE(a.UNSAFE_unverified() == 3);
  REQUIRE(b.UNSAFE_unverified() == 7);
  REQUIRE(c.UNSAFE_unverified() == 6);
  REQUIRE(d.UNSAFE_unverified() == 6);
  REQUIRE(e.UNSAFE_unverified() == 10);

  tainted<uint32_t, TestSandbox> ovWrap = std::numeric_limits<uint32_t>::max();
  ovWrap = ovWrap + 1;
  REQUIRE(ovWrap.UNSAFE_unverified() == 0);
}

struct test_tainted_struct_vals
{
  tainted<int32_t, TestSandbox> a{ 3 }; // NOLINT
  tainted<int32_t, TestSandbox> b{ 7 }; // NOLINT
};

// NOLINTNEXTLINE
TEST_CASE("Test operator + with const refs", "[operator]")
{
  test_tainted_struct_vals vals;
  const auto& ref = vals;
  REQUIRE((ref.a + ref.b).UNSAFE_unverified() == 10);
}

// NOLINTNEXTLINE
TEST_CASE("Test compound assignment operators", "[operator]")
{
  const int32_t a = 3;
  tainted<int32_t, TestSandbox> b = a;
  tainted<int32_t, TestSandbox> c = b;
  c += 1;
  tainted<int32_t, TestSandbox> d = b;
  REQUIRE(b.UNSAFE_unverified() == a);
  REQUIRE(c.UNSAFE_unverified() == a + 1);
  REQUIRE(d.UNSAFE_unverified() == a);
}

// NOLINTNEXTLINE
TEST_CASE("Test pre/post increment operators", "[operator]")
{
  const int32_t val = 3;

  SECTION("Test pre increment") // NOLINT
  {
    int32_t a = val;
    int32_t b = ++a;
    int32_t c = a;

    tainted<int32_t, TestSandbox> t_a = val;
    tainted<int32_t, TestSandbox> t_b = ++t_a;
    tainted<int32_t, TestSandbox> t_c = t_a;

    REQUIRE(t_a.UNSAFE_unverified() == a);
    REQUIRE(t_b.UNSAFE_unverified() == b);
    REQUIRE(t_c.UNSAFE_unverified() == c);
  }

  SECTION("Test post increment") // NOLINT
  {
    int32_t a = val;
    int32_t b = a++;
    int32_t c = a;

    tainted<int32_t, TestSandbox> t_a = val;
    tainted<int32_t, TestSandbox> t_b = t_a++;
    tainted<int32_t, TestSandbox> t_c = t_a;

    REQUIRE(t_a.UNSAFE_unverified() == a);
    REQUIRE(t_b.UNSAFE_unverified() == b);
    REQUIRE(t_c.UNSAFE_unverified() == c);
  }
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

  REQUIRE(s_r.UNSAFE_unverified() == r);
}

// NOLINTNEXTLINE
TEST_CASE("Test operator +, - for pointers", "[operator]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  tainted<int32_t*, TestSandbox> pc = sandbox.malloc_in_sandbox<int32_t>();
  tainted<int32_t*, TestSandbox> inc = pc + 1;

  auto diff = reinterpret_cast<char*>(inc.UNSAFE_unverified()) - // NOLINT
              reinterpret_cast<char*>(pc.UNSAFE_unverified());   // NOLINT
  REQUIRE(diff == 4);

  tainted<int32_t*, TestSandbox> nullPtr = nullptr;
  // operation on null pointer should throw
  REQUIRE_THROWS(nullPtr + 1);

  // pointer addition overflow sandbox bounds should throw
  REQUIRE_THROWS(pc + TestSandbox::SandboxMemorySize);

  tainted<int32_t*, TestSandbox> dec = inc - 1;
  REQUIRE(pc.UNSAFE_unverified() == dec.UNSAFE_unverified());

  auto pc2 = sandbox.malloc_in_sandbox<char>();
  auto inc2 = pc2 + 1;

  auto diff2 = reinterpret_cast<char*>(inc2.UNSAFE_unverified()) - // NOLINT
               reinterpret_cast<char*>(pc2.UNSAFE_unverified());   // NOLINT
  REQUIRE(diff2 == 1);

  auto pc3 = sandbox.malloc_in_sandbox<int32_t*>();
  auto inc3 = pc3 + 1;

  auto diff3 = reinterpret_cast<char*>(inc3.UNSAFE_unverified()) - // NOLINT
               reinterpret_cast<char*>(pc3.UNSAFE_unverified());   // NOLINT
  REQUIRE(diff3 == sizeof(TestSandbox::T_PointerType));

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE(
  "Test operators that produce new values for tainted_volatile numerics",
  "[operator]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
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
    REQUIRE(result.UNSAFE_unverified() == expected_result);
  }

  {
    tainted<uint64_t, TestSandbox> result = (*pc) + (*pc);
    uint64_t expected_result = max32Val + max32Val;
    REQUIRE(result.UNSAFE_unverified() == expected_result);
  }

  sandbox.destroy_sandbox();
}