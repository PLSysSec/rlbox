/**
 * @file test_tainted_equalities.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that equalities for various tainted wrappers work correctly.
 */

#include <type_traits>

#include "test_include.hpp"

#include "rlbox_tainted_hint.hpp"

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("tainted equalities operates correctly", "[tainted equality]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int> a = 34;

  REQUIRE(std::is_same_v<decltype(a == 34), bool>);
  REQUIRE(a == 34);
  REQUIRE(std::is_same_v<decltype(a != 0), bool>);
  REQUIRE(a != 0);

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

  REQUIRE(std::is_same_v<decltype(*a == 34),
                         tainted_boolean_hint<rlbox_sandbox_type_test>>);
  REQUIRE((*a == 34).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(*a != 0),
                         tainted_boolean_hint<rlbox_sandbox_type_test>>);
  REQUIRE((*a != 0).UNSAFE_unverified());

  REQUIRE(std::is_same_v<decltype(34 == *a),
                         tainted_boolean_hint<rlbox_sandbox_type_test>>);
  REQUIRE((34 == *a).UNSAFE_unverified());
  REQUIRE(std::is_same_v<decltype(0 != *a),
                         tainted_boolean_hint<rlbox_sandbox_type_test>>);
  REQUIRE((0 != *a).UNSAFE_unverified());

  sandbox.free_in_sandbox(a);
  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
