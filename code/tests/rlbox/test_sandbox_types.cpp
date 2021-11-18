#include "test_include.hpp"

RLBOX_DEFINE_BASE_TYPES_FOR(testlib, test);

// NOLINTNEXTLINE
TEST_CASE("Test rlbox type macro", "[rlbox types]")
{
  rlbox_sandbox_testlib sandbox;
  sandbox.create_sandbox();

  tainted_testlib<int> a = 4;

  sandbox.destroy_sandbox();

  // This is a compile time test so no requires.
}