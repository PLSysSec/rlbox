#include "test_include.hpp"

using RL = rlbox::RLBoxSandbox<rlbox::rlbox_noop_sandbox>;


void test_func()
{
}

// NOLINTNEXTLINE
TEST_CASE("sandbox_invoke on no_op sandbox", "[no_op_sandbox]")
{
    RL sandbox;
    sandbox.create_sandbox();

    REQUIRE_THROWS(sandbox_invoke(sandbox, test_func)); // NOLINT

    sandbox.destroy_sandbox();
}
