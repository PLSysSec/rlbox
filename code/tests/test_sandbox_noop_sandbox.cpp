// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol

#include "test_include.hpp"

using rlbox::rlbox_noop_sandbox;
using rlbox::tainted;
using RL = rlbox::RLBoxSandbox<rlbox_noop_sandbox>;

int GlobalVal = 0;
static void test_func_void(int param)
{
  GlobalVal = param;
}

static int test_func_int(int param)
{
  return param;
}

// NOLINTNEXTLINE
TEST_CASE("invoke in no_op sandbox", "[no_op_sandbox]")
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

using T_Func_int_int = int (*)(int);

static tainted<int, rlbox_noop_sandbox> test_callback(
  RL&, // NOLINT
  tainted<int, rlbox_noop_sandbox> val)
{
  return val + 1;
}

static int test_invoker(T_Func_int_int cb, int val)
{
  return (cb(val)) + 1;
}

// NOLINTNEXTLINE
TEST_CASE("callback in no_op sandbox", "[no_op_sandbox]")
{
  RL sandbox;
  sandbox.create_sandbox();

  rlbox::sandbox_callback<T_Func_int_int, rlbox_noop_sandbox> cb =
    sandbox.register_callback(test_callback);

  const int test_val = 5;
  tainted<int, rlbox_noop_sandbox> ret =
    sandbox_invoke(sandbox, test_invoker, cb, test_val); // NOLINT

  REQUIRE(ret.UNSAFE_Unverified() == test_val + 2);

  sandbox.destroy_sandbox();
}