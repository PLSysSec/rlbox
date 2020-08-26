// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
#define RLBOX_MEASURE_TRANSITION_TIMES
#include "rlbox_noop_sandbox.hpp"
#include "test_include.hpp"

#include <iostream>
#include <vector>

using RL = rlbox::rlbox_sandbox<rlbox::rlbox_noop_sandbox>;

static int add(int a, int b)
{
  return a + b;
}

// NOLINTNEXTLINE
TEST_CASE("sandbox timing tests", "[sandbox_timing_tests]")
{
  RL sandbox;
  sandbox.create_sandbox();

  SECTION("Per function test") // NOLINT
  {
    const int iterations = 10;
    const int val1 = 2;
    const int val2 = 3;
    for (int i = 0; i < iterations; i++) {
      auto result1 = sandbox.invoke_sandbox_function(add, val1, val2)
                       .unverified_safe_because("test");
      REQUIRE(result1 == val1 + val2);
    }

    auto& transition_times = sandbox.process_and_get_transition_times();
    REQUIRE(transition_times.size() == iterations);
    REQUIRE(sandbox.get_total_ns_time_in_sandbox_and_transitions() > 0);
  }

  sandbox.destroy_sandbox();
}