/**
 * @file test_tainted_equalities.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that equalities for various tainted wrappers work correctly.
 */

#include <type_traits>

#include "test_include.hpp"

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("tainted equalities operates correctly", "[tainted equality]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int> a = 34;
  tainted_test<int> b = 34;
  tainted_test<int> c = 2;

  // tainted with tainted
  REQUIRE(std::is_same_v<decltype(a == b), bool>);
  REQUIRE(a == b);
  REQUIRE(std::is_same_v<decltype(a != c), bool>);
  REQUIRE(a != c);

  // tainted with primitive
  REQUIRE(std::is_same_v<decltype(a == 34), bool>);
  REQUIRE(a == 34);
  REQUIRE(std::is_same_v<decltype(a != 0), bool>);
  REQUIRE(a != 0);

  // primitive with tainted
  REQUIRE(std::is_same_v<decltype(34 == a), bool>);
  REQUIRE(34 == a);
  REQUIRE(std::is_same_v<decltype(0 != a), bool>);
  REQUIRE(0 != a);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted volatile equalities operates correctly",
          "[tainted equality]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int*> a = sandbox.malloc_in_sandbox<int>();
  *a = 34;
  tainted_test<int*> b = sandbox.malloc_in_sandbox<int>();
  *b = 34;
  tainted_test<int*> c = sandbox.malloc_in_sandbox<int>();
  *c = 2;

  tainted_test<int> ta = 34;
  tainted_test<int> tb = 34;
  tainted_test<int> tc = 2;

  // tainted_volatile with tainted_volatile
  REQUIRE(std::is_same_v<decltype(*a == *b), tainted_boolean_hint_test>);
  REQUIRE((*a == *b).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(*a != *c), tainted_boolean_hint_test>);
  REQUIRE((*a != *c).UNSAFE_unverified());

  // tainted_volatile with tainted
  REQUIRE(std::is_same_v<decltype(*a == tb), tainted_boolean_hint_test>);
  REQUIRE((*a == tb).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(*a != tc), tainted_boolean_hint_test>);
  REQUIRE((*a != tc).UNSAFE_unverified());

  // tainted with tainted_volatile
  REQUIRE(std::is_same_v<decltype(ta == *b), tainted_boolean_hint_test>);
  REQUIRE((ta == *b).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(ta != *c), tainted_boolean_hint_test>);
  REQUIRE((ta != *c).UNSAFE_unverified());

  // tainted_volatile with primitive
  REQUIRE(std::is_same_v<decltype(*a == 34), tainted_boolean_hint_test>);
  REQUIRE((*a == 34).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(*a != 0), tainted_boolean_hint_test>);
  REQUIRE((*a != 0).UNSAFE_unverified());

  // primitive with tainted_volatile
  REQUIRE(std::is_same_v<decltype(34 == *a), tainted_boolean_hint_test>);
  REQUIRE((34 == *a).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(0 != *a), tainted_boolean_hint_test>);
  REQUIRE((0 != *a).UNSAFE_unverified());

  sandbox.free_in_sandbox(c);
  sandbox.free_in_sandbox(b);
  sandbox.free_in_sandbox(a);
  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
