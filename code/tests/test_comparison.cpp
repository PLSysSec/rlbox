#include <cstdint>

#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test comparisons to nullptr", "[compare_nullptr]")
{
  rlbox::RLBoxSandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
  REQUIRE(ptr != nullptr);
  REQUIRE(!(ptr == nullptr));
  REQUIRE(!!ptr);

  // Comparisons to nullptr not allowed for non pointers
  tainted<uint32_t, TestSandbox> val; // NOLINT
  REQUIRE_THROWS(val != nullptr);
  REQUIRE_THROWS(!(val == nullptr));

  sandbox.destroy_sandbox();
}
