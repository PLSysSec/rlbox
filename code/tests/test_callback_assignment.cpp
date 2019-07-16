#include "test_include.hpp"

using rlbox::tainted;
using RL = rlbox::RLBoxSandbox<TestSandbox>;

// NOLINTNEXTLINE
tainted<int, TestSandbox> test_cb(RL& /*unused*/, tainted<int, TestSandbox> a)
{
  return a;
}

// NOLINTNEXTLINE
TEST_CASE("Test sandbox_callback assignment", "[sandbox_callback]")
{
  RL sandbox;
  sandbox.create_sandbox();

  using T_F = int (*)(int);

  auto ptr = sandbox.malloc_in_sandbox<T_F>();
  auto cb = sandbox.register_callback(test_cb);

  tainted<T_F, TestSandbox> val = nullptr;

  // Assignment to tainted is not ok
  REQUIRE_COMPILE_ERR(val = cb);

  // Assignment to tainted_volatile is fine
  REQUIRE_NO_COMPILE_ERR(*ptr = cb);

  sandbox.destroy_sandbox();
}
