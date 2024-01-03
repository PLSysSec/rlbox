/**
 * @file test_forwarder.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that rlbox_forwarder works as expected.
 */

#include <optional>
#include <type_traits>
#include <utility>

#include "catch2/catch.hpp"

#include "rlbox_helpers.hpp"
#include "rlbox_type_traits.hpp"

class equals_test_subclass : public std::optional<int> {
 public:
#define RLBOX_FORWARD_TARGET_CLASS std::optional<int>
#define RLBOX_FORWARD_CURR_CLASS equals_test_subclass
#define RLBOX_FORWARD_TO_SUBCLASS

#include "rlbox_forwarder.hpp"
};

TEST_CASE("rlbox_forwarder operater (not) equals works correctly for subclass",
          "[rlbox_forwarder]") {
  std::optional<int> a1 = 3;
  std::optional<int> a2 = 4;

  int i1 = 3;
  int i2 = 4;

  equals_test_subclass f1 = 3;
  equals_test_subclass f1b = 3;
  equals_test_subclass f2 = 4;

  REQUIRE(f1 == f1b);

  REQUIRE(f1 == a1);
  REQUIRE(f1 == i1);

  REQUIRE(a1 == f1);
  REQUIRE(i1 == f1);

  REQUIRE(f1 != f2);

  REQUIRE(f1 != a2);
  REQUIRE(f1 != i2);

  REQUIRE(a1 != f2);
  REQUIRE(i1 != f2);
}

class equals_test_member {
 public:
  std::optional<int> m;
#define RLBOX_FORWARD_TARGET_CLASS std::optional<int>
#define RLBOX_FORWARD_CURR_CLASS equals_test_member
#define RLBOX_FORWARD_TO_OBJECT m

#include "rlbox_forwarder.hpp"
};

TEST_CASE("rlbox_forwarder operater (not) equals works correctly for member",
          "[rlbox_forwarder]") {
  std::optional<int> a1 = 3;
  std::optional<int> a2 = 4;

  int i1 = 3;
  int i2 = 4;

  equals_test_member f1 = 3;
  equals_test_member f1b = 3;
  equals_test_member f2 = 4;

  REQUIRE(f1 == f1b);

  REQUIRE(f1 == a1);
  REQUIRE(f1 == i1);

  REQUIRE(a1 == f1);
  REQUIRE(i1 == f1);

  REQUIRE(f1 != f2);

  REQUIRE(f1 != a2);
  REQUIRE(f1 != i2);

  REQUIRE(a1 != f2);
  REQUIRE(i1 != f2);
}
