#include <vector>

// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_test_sandbox_lookup_symbol
#include "test_include.hpp"

using rlbox::tainted;
using RL = rlbox::rlbox_sandbox<TestSandbox>;

static int test_fn(int a)
{
  return a;
}

// NOLINTNEXTLINE
TEST_CASE("Test sandbox_function assignment", "[sandbox_function]")
{
  RL sandbox;
  sandbox.create_sandbox();

  using T_F = int (*)(int);

  auto ptr = sandbox.malloc_in_sandbox<T_F>();
  auto cb = sandbox.get_sandbox_function_address(test_fn); // NOLINT

  tainted<T_F, TestSandbox> val = nullptr;

  // Assignment to another tainted is ok
  REQUIRE_NO_COMPILE_ERR(val = cb);

  // Assignment to tainted_volatile is fine
  REQUIRE_NO_COMPILE_ERR(*ptr = cb);

  sandbox.destroy_sandbox();
}
