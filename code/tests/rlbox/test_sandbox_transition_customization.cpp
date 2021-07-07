// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
namespace rlbox {
enum class rlbox_transition;
}
void on_transition_in(rlbox::rlbox_transition transition,
                      const char* func_name,
                      const void* func_ptr,
                      void* saved_state);
void on_transition_out(rlbox::rlbox_transition transition,
                       const char* func_name,
                       const void* func_ptr,
                       void* saved_state);
#define RLBOX_TRANSITION_ACTION_IN(transition, func_name, func_ptr, state)     \
  on_transition_in(transition, func_name, func_ptr, state)
#define RLBOX_TRANSITION_ACTION_OUT(transition, func_name, func_ptr, state)    \
  on_transition_out(transition, func_name, func_ptr, state)
#include "test_include.hpp"

using RL = rlbox::rlbox_sandbox<rlbox::rlbox_noop_sandbox>;

static int add(int a, int b)
{
  return a + b;
}

static int transition_in_count = 0;
static int transition_out_count = 0;

void on_transition_in(rlbox::rlbox_transition transition,
                      const char* func_name,
                      const void* func_ptr,
                      void* saved_state)
{
  UNUSED(transition);
  UNUSED(func_name);
  UNUSED(func_ptr);
  UNUSED(saved_state);
  transition_in_count++;
}

void on_transition_out(rlbox::rlbox_transition transition,
                       const char* func_name,
                       const void* func_ptr,
                       void* saved_state)
{
  UNUSED(transition);
  UNUSED(func_name);
  UNUSED(func_ptr);
  UNUSED(saved_state);
  transition_out_count++;
}

// NOLINTNEXTLINE
TEST_CASE("sandbox transition customization",
          "[sandbox_transition_customization]")
{
  RL sandbox;
  sandbox.create_sandbox();

  transition_in_count = 0;
  transition_out_count = 0;

  const int val1 = 2;
  const int val2 = 3;
  sandbox.invoke_sandbox_function(add, val1, val2);
  REQUIRE(transition_in_count == 1);
  REQUIRE(transition_out_count == 1);

  sandbox.destroy_sandbox();
}