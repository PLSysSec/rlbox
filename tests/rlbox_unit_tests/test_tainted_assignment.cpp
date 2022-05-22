/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers can be assigned values and
 * correctly convert.
 */

#include "test_include.hpp"

#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_tainted_relocatable.hpp"

using rlbox::tainted_fixed_aligned;
using rlbox::tainted_relocatable;
// using rlbox::tainted_volatile;

template <template <typename, typename...> class TWrap>
static void test_tainted_helper() {
  const int random_val_1 = 4;
  const int random_val_2 = 5;

  TWrap<int, rlbox_test_sandbox> a{};
  a = random_val_1;
  TWrap<int, rlbox_test_sandbox> b = random_val_2;
  REQUIRE(a.UNSAFE_unverified() == random_val_1);
  REQUIRE(b.UNSAFE_unverified() == random_val_2);

  TWrap<int, rlbox_test_sandbox> c = b;
  TWrap<int, rlbox_test_sandbox> d{};
  d = b;
  REQUIRE(c.UNSAFE_unverified() == random_val_2);
  REQUIRE(d.UNSAFE_unverified() == random_val_2);

  TWrap<long, rlbox_test_sandbox> e = random_val_1;
  TWrap<int, rlbox_test_sandbox> f = e;
  TWrap<long, rlbox_test_sandbox> g = f;
  REQUIRE(e.UNSAFE_unverified() == random_val_1);
  REQUIRE(f.UNSAFE_unverified() == random_val_1);
  REQUIRE(g.UNSAFE_unverified() == random_val_1);

  const float float_val_1 = 2.4;
  TWrap<float, rlbox_test_sandbox> h = float_val_1;
  REQUIRE(h.UNSAFE_unverified() == float_val_1);
}

TEST_CASE("tainted assignment operates correctly", "[tainted_assignment]") {
  test_tainted_helper<tainted_fixed_aligned>();
  test_tainted_helper<tainted_relocatable>();
}

// // NOLINTNEXTLINE
// TEST_CASE("tainted_volatile assignment operates correctly",
//           "[tainted_assignment]")
// {
//   rlbox::rlbox_sandbox<rlbox_test_sandbox> sandbox;
//   sandbox.create_sandbox();

//   // On 64 bit platforms, "unsigned long" is 64 bits in the app
//   // but unsigned long is 32-bits in our test sandbox env
//   // NOLINTNEXTLINE(google-runtime-int)
//   auto pc = sandbox.malloc_in_sandbox<unsigned long>();

//   // Only run this test for platforms where unsigned long is 64 bits
//   // NOLINTNEXTLINE(google-runtime-int)
//   if constexpr (sizeof(unsigned long) == sizeof(uint64_t)) {
//     uint64_t max32Val = std::numeric_limits<uint32_t>::max();
//     *pc = max32Val;

//     REQUIRE((*pc).UNSAFE_unverified() == max32Val);
//     REQUIRE(pc->UNSAFE_unverified() == max32Val);

//     uint64_t max64Val = std::numeric_limits<uint64_t>::max();
//     REQUIRE_THROWS(*pc = max64Val);
//   }

//   sandbox.destroy_sandbox();
// }

// // NOLINTNEXTLINE
// TEST_CASE("tainted tainted_volatile conversion operates correctly",
//           "[tainted_assignment]")
// {
//   rlbox::rlbox_sandbox<rlbox_test_sandbox> sandbox;
//   sandbox.create_sandbox();

//   auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
//   REQUIRE(std::is_same_v<decltype(ptr), tainted<uint32_t*,
//   rlbox_test_sandbox>>); REQUIRE(ptr.UNSAFE_unverified() != nullptr);

//   auto& val = *ptr;
//   REQUIRE(
//     std::is_same_v<decltype(val), tainted_volatile<uint32_t,
//     rlbox_test_sandbox>&>);
//   REQUIRE(
//     std::is_same_v<decltype(tainted(val)), tainted<uint32_t,
//     rlbox_test_sandbox>>);

//   REQUIRE(
//     std::is_same_v<decltype(tainted(&val)), tainted<uint32_t*,
//     rlbox_test_sandbox>>);
//   REQUIRE(
//     std::is_same_v<decltype(tainted(&*ptr)), tainted<uint32_t*,
//     rlbox_test_sandbox>>);

//   tainted<uint32_t**, rlbox_test_sandbox> ptr2 =
//     sandbox.malloc_in_sandbox<uint32_t*>();
//   auto& deref = *ptr2;
//   REQUIRE(
//     std::is_same_v<decltype(deref), tainted_volatile<uint32_t*,
//     rlbox_test_sandbox>&>);
//   REQUIRE(
//     std::is_same_v<decltype(*deref), tainted_volatile<uint32_t,
//     rlbox_test_sandbox>&>);

//   REQUIRE(
//     std::is_same_v<decltype(**ptr2), tainted_volatile<uint32_t,
//     rlbox_test_sandbox>&>);

//   sandbox.destroy_sandbox();
// }