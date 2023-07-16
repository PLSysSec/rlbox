/**
 * @file test_create_destroy.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that sandbox create and destroy for each plugin works as
 * expected.
 */

#include "test_plugin_include.hpp"

TEST_CASE(TestName ": Create and destroy sandbox", "[create destroy]") {
  rlbox_sandbox<TestType> s;
  rlbox_status_code result = CreateSandbox(s);
  REQUIRE(result == rlbox_status_code::SUCCESS);

  rlbox_status_code result2 = s.destroy_sandbox();
  REQUIRE(result2 == rlbox_status_code::SUCCESS);
}

TEST_CASE(TestName ": Create and destroy multiple sandbox",
          "[create destroy]") {
  const int sandbox_count = 2;
  rlbox_sandbox<TestType> s[sandbox_count];

  for (int i = 0; i < sandbox_count; i++) {
    rlbox_status_code result = CreateSandbox(s[i]);
    REQUIRE(result == rlbox_status_code::SUCCESS);
  }

  for (int i = 0; i < sandbox_count; i++) {
    rlbox_status_code result2 = s[i].destroy_sandbox();
    REQUIRE(result2 == rlbox_status_code::SUCCESS);
  }
}