/**
 * @file test_memory_mgmt.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check the rlbox_unique_ptr works as expected
 */

#include "test_include.hpp"

TEST_CASE("Test functionality of rlbox_unique_ptr", "[sandbox_invoke]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  rlbox_unique_ptr_test<int> u1 = make_unique_tainted<int>(sandbox);
  *u1 = 4;
  REQUIRE((*u1.get()).UNSAFE_unverified() == 4);
  u1 = nullptr;

  rlbox_unique_ptr_test<int> u2 = make_unique_tainted<int>(sandbox);
  u2.swap(u1);

  u1 = nullptr;

  REQUIRE(u1.get() == nullptr);
  REQUIRE(u2.get() == nullptr);

  sandbox.destroy_sandbox();
}
