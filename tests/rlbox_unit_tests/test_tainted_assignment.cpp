/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers can be assigned values and
 * correctly convert.
 */

#include <type_traits>

#include "test_include.hpp"

#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_tainted_relocatable.hpp"

using rlbox::tainted_fixed_aligned;
using rlbox::tainted_relocatable;
// using rlbox::tainted_volatile;

/**
 * @brief Sandbox we will use for rlbox testing assignments. The type `short`
 * inside the sandbox is a different length
 */
class rlbox_diffabi_sandbox
    : public rlbox::rlbox_sandbox_plugin_base<rlbox_diffabi_sandbox> {
 public:
  using sbx_short = int8_t;
};

template <template <typename, typename...> class TWrap>
static void test_tainted_helper() {
  const int random_val_1 = 4;
  const int random_val_2 = 5;

  // Check basic assignments and initialization
  {
    TWrap<int, rlbox_diffabi_sandbox> a{};
    a = random_val_1;
    TWrap<int, rlbox_diffabi_sandbox> b = random_val_2;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_2);
  }

  // Check assignment from same wrapper type
  {
    TWrap<int, rlbox_diffabi_sandbox> a = random_val_1;
    TWrap<int, rlbox_diffabi_sandbox> b = a;
    TWrap<int, rlbox_diffabi_sandbox> c{};
    c = a;
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
    REQUIRE(c.UNSAFE_unverified() == random_val_1);
  }

  // Check assignments from compatible types
  {
    TWrap<long, rlbox_diffabi_sandbox> a = random_val_1;
    TWrap<int, rlbox_diffabi_sandbox> b = a;
    TWrap<long, rlbox_diffabi_sandbox> c = b;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
    REQUIRE(c.UNSAFE_unverified() == random_val_1);
  }

  // Check const assignment
  {
    TWrap<const int, rlbox_diffabi_sandbox> a = random_val_1;
    TWrap<const int, rlbox_diffabi_sandbox> b = a;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_sandboxed() == random_val_1);
  }

  // Check assignment overflow due to truncated sandbox type
  {
    const short overflow_val_1 = 400; /* sandbox short limit is uint8_t */
    TWrap<short, rlbox_diffabi_sandbox> a = overflow_val_1;
    // Converting to the sandbox repr should cause an error due to overflow
    REQUIRE_THROWS(a.UNSAFE_sandboxed() == overflow_val_1);
  }

  // Check floating point values
  {
    const float float_val_1 = 2.4;
    TWrap<float, rlbox_diffabi_sandbox> h = float_val_1;
    REQUIRE(h.UNSAFE_unverified() == float_val_1);
  }
}

TEST_CASE("tainted assignment operates correctly", "[tainted_assignment]") {
  test_tainted_helper<tainted_fixed_aligned>();
  test_tainted_helper<tainted_relocatable>();
}

TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted_assignment]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int*> ptr = sandbox.malloc_in_sandbox<int>();
  REQUIRE(std::is_base_of_v<tainted_base<int*, rlbox_sandbox_type_test>,
                            decltype(ptr)>);
  REQUIRE(ptr.UNSAFE_unverified() != nullptr);

  auto& val = *ptr;
  REQUIRE(std::is_same_v<decltype(val), tainted_volatile_test<int>&>);

  REQUIRE(std::is_same_v<decltype(tainted_test<int>(val)), tainted_test<int>>);

  [[maybe_unused]] auto val2 = tainted_test<int*>(&val);
  REQUIRE(std::is_same_v<decltype(val2), tainted_test<int*>>);
  REQUIRE(
      std::is_same_v<decltype(tainted_test<int*>(&*ptr)), tainted_test<int*>>);

  tainted_test<int**> ptr2 = sandbox.malloc_in_sandbox<int*>();
  auto& deref = *ptr2;
  REQUIRE(std::is_same_v<decltype(deref), tainted_volatile_test<int*>&>);
  REQUIRE(std::is_same_v<decltype(*deref), tainted_volatile_test<int>&>);

  REQUIRE(std::is_same_v<decltype(**ptr2), tainted_volatile_test<int>&>);

  sandbox.free_in_sandbox(ptr);

  sandbox.destroy_sandbox();
}