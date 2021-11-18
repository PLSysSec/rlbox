#include "test_include.hpp"

#include <memory>

using rlbox::tainted;
using rlbox::tainted_opaque;
using RL = rlbox::rlbox_sandbox<TestSandbox>;

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

// No sandbox as the first param
static void bad_callback_1() {}

// No sandbox as the first param
static void bad_callback_2(int) {} // NOLINT

// No tainted param
static void bad_callback_3(RL&, int) {} // NOLINT

// No tainted param
static tainted<int, TestSandbox> bad_callback_4(RL&, int) // NOLINT
{
  const int test_val = 5;
  return test_val;
}

// No tainted return
static int bad_callback_5(RL&) // NOLINT
{
  const int test_val = 5;
  return test_val;
}

// No tainted return
static int bad_callback_6(RL&, tainted<int, TestSandbox>) // NOLINT
{
  const int test_val = 5;
  return test_val;
}

// Using static array parameter
static void bad_callback_7(RL&, tainted<int[4], TestSandbox>) {} // NOLINT

static void good_callback_1(RL&) {} // NOLINT

static void good_callback_2(RL&, tainted<int, TestSandbox>) {} // NOLINT

static tainted<int, TestSandbox> good_callback_3(RL&) // NOLINT
{
  const int test_val = 5;
  return test_val;
}

static tainted<int, TestSandbox> good_callback_4(
  RL&,                       // NOLINT
  tainted<int, TestSandbox>) // NOLINT
{
  const int test_val = 5;
  return test_val;
}

static tainted_opaque<int, TestSandbox> good_callback_5(
  RL&,                              // NOLINT
  tainted_opaque<int, TestSandbox>) // NOLINT
{
  const int test_val = 5;
  return tainted<int, TestSandbox>(test_val).to_opaque();
}

// NOLINTNEXTLINE
TEST_CASE("callback sig checks", "[sandbox_callback]")
{
  RL sandbox;
  sandbox.create_sandbox();

  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_1));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_2));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_3));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_4));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_5));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_6));
  REQUIRE_COMPILE_ERR(sandbox.register_callback(bad_callback_7));
  REQUIRE_NO_COMPILE_ERR(sandbox.register_callback(good_callback_1));
  REQUIRE_NO_COMPILE_ERR(sandbox.register_callback(good_callback_2));
  REQUIRE_NO_COMPILE_ERR(sandbox.register_callback(good_callback_3));
  REQUIRE_NO_COMPILE_ERR(sandbox.register_callback(good_callback_4));
  REQUIRE_NO_COMPILE_ERR(sandbox.register_callback(good_callback_5));

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("callback assignment check", "[sandbox_callback]")
{
  RL sandbox;
  sandbox.create_sandbox();

  auto p_fnPtr = sandbox.malloc_in_sandbox<int (*)(int)>();
  REQUIRE_NO_COMPILE_ERR(p_fnPtr = nullptr);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("callback re-register", "[sandbox_callback]")
{
  RL sandbox;
  sandbox.create_sandbox();

  using T_F = int (*)(int);
  {
    auto cb = std::make_unique<rlbox::sandbox_callback<T_F, TestSandbox>>(
      sandbox.register_callback(test_cb));
  }
  {
    auto cb2 = sandbox.register_callback(test_cb);
  }
  {
    auto cb3 = sandbox.register_callback(test_cb);
  }

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("callback is-registered", "[sandbox_callback]")
{
  RL sandbox;
  sandbox.create_sandbox();

  using T_F = int (*)(int);
  auto cb = std::make_unique<rlbox::sandbox_callback<T_F, TestSandbox>>(
    sandbox.register_callback(test_cb));

  REQUIRE(!cb->is_unregistered());
  cb->unregister();
  REQUIRE(cb->is_unregistered());

  sandbox.destroy_sandbox();
}
