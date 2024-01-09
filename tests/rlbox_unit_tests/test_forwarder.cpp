/**
 * @file test_forwarder.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that rlbox_forwarder works as expected.
 */

#include <optional>
// NOLINTBEGIN(misc-include-cleaner)
// Needed by rlbox_forwarder.hpp
#include <type_traits>
#include <utility>

#include "rlbox_helpers.hpp"
#include "rlbox_type_traits.hpp"
// NOLINTEND(misc-include-cleaner)

#include "catch2/catch.hpp"

class optional_test_subclass : public std::optional<int> {
 public:
#define RLBOX_FORWARD_TARGET_CLASS std::optional<int>
#define RLBOX_FORWARD_CURR_CLASS optional_test_subclass
#define RLBOX_FORWARD_TO_SUBCLASS

#include "rlbox_forwarder.hpp"
};

class optional_test_member {
 public:
  std::optional<int> m;
#define RLBOX_FORWARD_TARGET_CLASS std::optional<int>
#define RLBOX_FORWARD_CURR_CLASS optional_test_member
#define RLBOX_FORWARD_TO_OBJECT m

#include "rlbox_forwarder.hpp"
};

TEMPLATE_TEST_CASE("rlbox_forwarder constructor works correctly",
                   "[rlbox_forwarder]", optional_test_subclass,
                   optional_test_member) {
  [[maybe_unused]] TestType t;
}

// NOLINTBEGIN(misc-const-correctness)

TEMPLATE_TEST_CASE("rlbox_forwarder operater (not) equals works correctly",
                   "[rlbox_forwarder]", optional_test_subclass,
                   optional_test_member) {
  std::optional<int> a1 = 3;
  std::optional<int> a2 = 4;

  int i1 = 3;
  int i2 = 4;

  TestType f1 = 3;
  TestType f1b = 3;
  TestType f2 = 4;

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

// NOLINTEND(misc-const-correctness)
