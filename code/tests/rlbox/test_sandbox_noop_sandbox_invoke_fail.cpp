#define RLBOX_NO_COMPILE_CHECKS
#define RLBOX_USE_EXCEPTIONS
#include "rlbox_noop_sandbox.hpp" // IWYU pragma: keep
#include "test_include.hpp"

using RL = rlbox::rlbox_sandbox<rlbox::rlbox_noop_sandbox>;

static void test_func() {}

// This test has to be in a separate file as it is testing the missing #define
// before the #include "rlbox_noop_sandbox.hpp"
// NOLINTNEXTLINE
TEST_CASE("sandbox_lookup_symbol on no_op sandbox without #define causes error",
          "[no_op_sandbox]")
{
  RL sandbox;
  sandbox.create_sandbox();

  // Error due to the missing #define described above
  REQUIRE_COMPILE_ERR(sandbox.sandbox_invoke(test_func)); // NOLINT
  UNUSED(test_func);

  sandbox.destroy_sandbox();
}
