/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers can be assigned values.
 */

#include "test_include.hpp"

#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_tainted_relocatable.hpp"

// NOLINTBEGIN(misc-const-correctness)

static void test_tainted_helper() {
  const int random_val_1 = 4;
  const int random_val_2 = 5;

  // Check basic assignments and initialization
  {
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> a{};
    a = random_val_1;
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> b =
        random_val_2;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_2);
  }

  // Check assignment from same wrapper type
  {
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> a =
        random_val_1;
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> b = a;
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> c{};
    c = a;
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
    REQUIRE(c.UNSAFE_unverified() == random_val_1);
  }

  // Check assignments from compatible types
  {
    tainted_impl<true, long, rlbox_noop_arena_smallerabi_sandbox> a =
        random_val_1;
    tainted_impl<true, int, rlbox_noop_arena_smallerabi_sandbox> b = a;
    tainted_impl<true, long, rlbox_noop_arena_smallerabi_sandbox> c = b;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
    REQUIRE(c.UNSAFE_unverified() == random_val_1);
  }

  // Check const assignment
  {
    tainted_impl<true, const int, rlbox_noop_arena_smallerabi_sandbox> a =
        random_val_1;
    tainted_impl<true, const int, rlbox_noop_arena_smallerabi_sandbox> b = a;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_sandboxed() == random_val_1);
  }

  // Check assignment overflow due to truncated sandbox type
  {
    const short overflow_val_1 = 400; /* sandbox short limit is uint8_t */
    tainted_impl<true, short, rlbox_noop_arena_smallerabi_sandbox> a =
        overflow_val_1;
    // Converting to the sandbox repr should cause an error due to overflow
    REQUIRE_THROWS(a.UNSAFE_sandboxed() == overflow_val_1);
  }

  // Check floating point values
  {
    const float float_val_1 = 2.4;
    tainted_impl<true, float, rlbox_noop_arena_smallerabi_sandbox> a =
        float_val_1;
    REQUIRE(a.UNSAFE_unverified() == float_val_1);
  }
}

TEST_CASE("tainted assignment operates correctly", "[tainted assignment]") {
  test_tainted_helper();
  // test_tainted_helper<tainted_relocatable>();
}

TEST_CASE("tainted volatile assignment operates correctly",
          "[tainted assignment]") {
  rlbox_sandbox_test_smallerabi sandbox;
  sandbox.create_sandbox();

  tainted_test_smallerabi<int*> ptr_int_1 = sandbox.malloc_in_sandbox<int>();
  tainted_test_smallerabi<int*> ptr_int_2 = sandbox.malloc_in_sandbox<int>();
  tainted_test_smallerabi<long*> ptr_long_1 = sandbox.malloc_in_sandbox<long>();
  tainted_test_smallerabi<long*> ptr_long_2 = sandbox.malloc_in_sandbox<long>();
  tainted_test_smallerabi<short*> ptr_short_1 =
      sandbox.malloc_in_sandbox<short>();
  tainted_test_smallerabi<float*> ptr_float_1 =
      sandbox.malloc_in_sandbox<float>();

  const int random_val_1 = 4;
  const int random_val_2 = 5;

  // Check basic assignments and initialization
  {
    tainted_volatile_test_smallerabi<int> a = *ptr_int_1;
    tainted_volatile_test_smallerabi<int> b = *ptr_int_2;
    a = random_val_1;
    b = random_val_2;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_2);
  }

  // Check assignment from same wrapper type
  {
    tainted_volatile_test_smallerabi<int> a = *ptr_int_1;
    tainted_volatile_test_smallerabi<int> b = *ptr_int_2;

    a = random_val_1;
    b = a;
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
  }

  // Check assignments from compatible types
  {
    tainted_volatile_test_smallerabi<long> a = *ptr_long_1;
    tainted_volatile_test_smallerabi<int> b = *ptr_int_2;
    tainted_volatile_test_smallerabi<long> c = *ptr_long_2;

    a = random_val_1;
    b = a;
    c = b;
    REQUIRE(a.UNSAFE_unverified() == random_val_1);
    REQUIRE(b.UNSAFE_unverified() == random_val_1);
    REQUIRE(c.UNSAFE_unverified() == random_val_1);
  }

  // Check assignment overflow due to truncated sandbox type
  {
    tainted_volatile_test_smallerabi<short> a = *ptr_short_1;

    const short overflow_val_1 = 400; /* sandbox short limit is uint8_t */
    // Converting to the sandbox repr should cause an error due to overflow
    REQUIRE_THROWS(a = overflow_val_1);
  }

  // Check floating point values
  {
    tainted_volatile_test_smallerabi<float> a = *ptr_float_1;

    const float float_val_1 = 2.4;
    a = float_val_1;
    REQUIRE(a.UNSAFE_unverified() == float_val_1);
  }

  sandbox.free_in_sandbox(ptr_float_1);
  sandbox.free_in_sandbox(ptr_short_1);
  sandbox.free_in_sandbox(ptr_long_2);
  sandbox.free_in_sandbox(ptr_long_1);
  sandbox.free_in_sandbox(ptr_int_2);
  sandbox.free_in_sandbox(ptr_int_1);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted pointers assignment operates correctly",
          "[tainted assignment]") {
  tainted_impl<true, int*, rlbox_noop_arena_smallerabi_sandbox>
      ptr_taint_fixed = nullptr;
  ptr_taint_fixed = nullptr;
  // tainted_relocatable<int*, rlbox_noop_arena_smallerabi_sandbox>
  // ptr_taint_reloc = nullptr; ptr_taint_reloc = nullptr;

  rlbox_sandbox_test_smallerabi sandbox;
  sandbox.create_sandbox();
  tainted_test_smallerabi<int**> pp_int_taint_vol =
      sandbox.malloc_in_sandbox<int*>();

  *pp_int_taint_vol = nullptr;

  sandbox.free_in_sandbox(pp_int_taint_vol);
  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
