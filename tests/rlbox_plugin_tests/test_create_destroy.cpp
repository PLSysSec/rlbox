/**
 * @file test_create_destroy.cpp
 * @brief Check that sandbox create and destroy for each plugin works as
 * expected.
 */

#include "test_plugin_include.hpp"

TEST_CASE("Create and destroy sandbox" TestName, "[create destroy]")
{
  rlbox_sandbox<TestType> s;
  rlbox_status_code result = CreateSandbox(s);
  REQUIRE(result == rlbox_status_code::SUCCESS);

  rlbox_status_code result2 = s.destroy_sandbox();
  REQUIRE(result2 == rlbox_status_code::SUCCESS);
}

TEST_CASE("Create and destroy multiple sandbox" TestName, "[create destroy]")
{
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