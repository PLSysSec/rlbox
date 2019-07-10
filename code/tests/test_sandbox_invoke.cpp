// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_no_op_sandbox_lookup_symbol

#include "rlbox_noop_sandbox.hpp"
#include "test_include.hpp"

using RL = rlbox::RLBoxSandbox<rlbox::rlbox_noop_sandbox>;

int GlobalVal = 0;
void test_func_void(int param)
{
  GlobalVal = param;
}

int test_func_int(int param)
{
  return param;
}

// NOLINTNEXTLINE
TEST_CASE("sandbox_invoke on no_op sandbox", "[no_op_sandbox]")
{
  RL sandbox;
  sandbox.create_sandbox();

  void* a = sandbox_lookup_symbol(sandbox, test_func_void); // NOLINT
  REQUIRE(a == reinterpret_cast<void*>(&test_func_void));   // NOLINT

  const int TestFuncVal = 3;
  sandbox_invoke(sandbox, test_func_void, TestFuncVal); // NOLINT
  REQUIRE(GlobalVal == TestFuncVal);                    // NOLINT

  auto result = sandbox_invoke(sandbox, test_func_int, TestFuncVal); // NOLINT
  REQUIRE(result.UNSAFE_Unverified() == TestFuncVal);                // NOLINT

  sandbox.destroy_sandbox();
}
