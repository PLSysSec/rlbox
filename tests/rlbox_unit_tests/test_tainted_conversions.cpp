/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers correctly convert.
 */

#include <type_traits>

#include "test_include.hpp"

TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted tainted_volatile conversion]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int*> ptr = sandbox.malloc_in_sandbox<int>();
  REQUIRE(std::is_base_of_v<tainted_interface<rlbox_sandbox_type_test>,
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
