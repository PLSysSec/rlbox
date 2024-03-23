/**
 * @file test_operators.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that operators on tainted works as expected
 */

#include <limits>
#include <stdint.h>

#include "rlbox_stdint_types.hpp"

#include "test_include.hpp"

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("Test operator + for numerics", "[operator]") {
  tainted_test<int32_t> a = 3;
  tainted_test<int32_t> b = 3 + 4;
  tainted_test<int32_t> c = a + 3;
  tainted_test<int32_t> d = 3 + a;
  tainted_test<int32_t> e = a + b;
  REQUIRE(a.UNSAFE_unverified() == 3);
  REQUIRE(b.UNSAFE_unverified() == 7);
  REQUIRE(c.UNSAFE_unverified() == 6);
  REQUIRE(d.UNSAFE_unverified() == 6);
  REQUIRE(e.UNSAFE_unverified() == 10);

  tainted_test<rlbox_uint32_t> ov_wrap = std::numeric_limits<uint32_t>::max();
  ov_wrap = ov_wrap + 1;
  REQUIRE(ov_wrap.UNSAFE_unverified() == 0);
}

// struct test_tainted_struct_vals
// {
//   tainted_test<int32_t> a{ 3 }; // NOLINT
//   tainted_test<int32_t> b{ 7 }; // NOLINT
// };

// TEST_CASE("Test operator + with const refs", "[operator]")
// {
//   test_tainted_struct_vals vals;
//   const auto& ref = vals;
//   REQUIRE((ref.a + ref.b).UNSAFE_unverified() == 10);
// }

TEST_CASE("Test compound assignment operators", "[operator]") {
  const int32_t a = 3;
  tainted_test<int32_t> b = a;
  tainted_test<int32_t> c = b;
  c += 1;
  c += b;
  tainted_test<int32_t> d = b;
  REQUIRE(b.UNSAFE_unverified() == a);
  REQUIRE(c.UNSAFE_unverified() == a + 1 + 3);
  REQUIRE(d.UNSAFE_unverified() == a);
}

TEST_CASE("Test pre/post increment operators", "[operator]") {
  const int32_t val = 3;

  SECTION("Test pre increment")  // NOLINT
  {
    int32_t a = val;
    int32_t b = ++a;
    int32_t c = a;

    tainted_test<int32_t> t_a = val;
    tainted_test<int32_t> t_b = ++t_a;
    tainted_test<int32_t> t_c = t_a;

    REQUIRE(t_a.UNSAFE_unverified() == a);
    REQUIRE(t_b.UNSAFE_unverified() == b);
    REQUIRE(t_c.UNSAFE_unverified() == c);
  }

  SECTION("Test post increment")  // NOLINT
  {
    int32_t a = val;
    int32_t b = a++;
    int32_t c = a;

    tainted_test<int32_t> t_a = val;
    tainted_test<int32_t> t_b = t_a++;
    tainted_test<int32_t> t_c = t_a;

    REQUIRE(t_a.UNSAFE_unverified() == a);
    REQUIRE(t_b.UNSAFE_unverified() == b);
    REQUIRE(t_c.UNSAFE_unverified() == c);
  }
}

TEST_CASE("Test operators that produce new values for numerics", "[operator]") {
  const uint32_t a = 11;
  const uint32_t b = 17;
  const uint32_t c = 13;
  const uint32_t d = 17;
  const uint32_t e = 2;
  uint32_t r = -(((((a + b) - c) * d) / e));

  tainted_test<uint32_t> s_a = a;
  tainted_test<uint32_t> s_b = b;
  tainted_test<uint32_t> s_c = c;
  tainted_test<uint32_t> s_d = d;
  tainted_test<uint32_t> s_e = e;
  tainted_test<uint32_t> s_r = -(((((s_a + s_b) - s_c) * s_d) / s_e));

  REQUIRE(s_r.UNSAFE_unverified() == r);
}

// TEST_CASE("Test operator +, - for pointers", "[operator]")
// {
//   rlbox_sandbox_test sandbox;
//   sandbox.create_sandbox();

//   tainted_test<int32_t*> pc = sandbox.malloc_in_sandbox<int32_t>();
//   tainted_test<int32_t*> inc = pc + 1;

//   auto diff = reinterpret_cast<char*>(inc.UNSAFE_unverified()) - // NOLINT
//               reinterpret_cast<char*>(pc.UNSAFE_unverified());   // NOLINT
//   REQUIRE(diff == 4);

//   tainted_test<int32_t*> nullPtr = nullptr;
//   // operation on null pointer should throw
//   REQUIRE_THROWS(nullPtr + 1);

//   // pointer addition overflow sandbox bounds should throw
//   REQUIRE_THROWS(pc + TestSandbox::SandboxMemorySize);

//   tainted_test<int32_t*> dec = inc - 1;
//   REQUIRE(pc.UNSAFE_unverified() == dec.UNSAFE_unverified());

//   auto pc2 = sandbox.malloc_in_sandbox<char>();
//   auto inc2 = pc2 + 1;

//   auto diff2 = reinterpret_cast<char*>(inc2.UNSAFE_unverified()) - // NOLINT
//                reinterpret_cast<char*>(pc2.UNSAFE_unverified());   // NOLINT
//   REQUIRE(diff2 == 1);

//   auto pc3 = sandbox.malloc_in_sandbox<int32_t*>();
//   auto inc3 = pc3 + 1;

//   auto diff3 = reinterpret_cast<char*>(inc3.UNSAFE_unverified()) - // NOLINT
//                reinterpret_cast<char*>(pc3.UNSAFE_unverified());   // NOLINT
//   REQUIRE(diff3 == sizeof(TestSandbox::T_PointerType));

//   sandbox.destroy_sandbox();
// }

TEST_CASE(
    "Test operators that produce new values for tainted_volatile numerics",
    "[operator]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  // uint64_t on 64 bit platforms is "unsigned long" which is 64 bits in the app
  // but long is 16-bits in our test sandbox env
  auto pc = sandbox.malloc_in_sandbox<uint64_t>();

  uint64_t max32_val = std::numeric_limits<uint32_t>::max();
  REQUIRE_THROWS(*pc = max32_val);

  uint64_t max16_val = std::numeric_limits<uint16_t>::max();
  *pc = max16_val;

  const int rhs = 1;
  {
    tainted_test<uint64_t> result = (*pc) + rhs;
    uint64_t expected_result = max16_val + rhs;
    REQUIRE(result.UNSAFE_unverified() == expected_result);
  }

  {
    tainted_test<uint64_t> result = (*pc) + (*pc);
    uint64_t expected_result = max16_val + max16_val;
    REQUIRE(result.UNSAFE_unverified() == expected_result);
  }

  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
