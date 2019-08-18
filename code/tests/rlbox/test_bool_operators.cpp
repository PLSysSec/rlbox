#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test bool operators", "[operator]")
{
  tainted<bool, TestSandbox> t = true;
  tainted<bool, TestSandbox> f = false;

  SECTION("tainted left") // NOLINT
  {
    REQUIRE(t.UNSAFE_unverified());

    REQUIRE_FALSE(f.UNSAFE_unverified());

    REQUIRE((!f).UNSAFE_unverified());

    tainted<bool, TestSandbox> and_false = f && true;
    REQUIRE_FALSE(and_false.UNSAFE_unverified());

    tainted<bool, TestSandbox> and_true = t && true;
    REQUIRE(and_true.UNSAFE_unverified());

    REQUIRE_FALSE((t&&f).UNSAFE_unverified());
    REQUIRE((t||f).UNSAFE_unverified());
  }

  SECTION("tainted right") // NOLINT
  {
    tainted<bool, TestSandbox> and_false = true && f;
    REQUIRE_FALSE(and_false.UNSAFE_unverified());

    tainted<bool, TestSandbox> and_true = true && t;
    REQUIRE(and_true.UNSAFE_unverified());
  }
}
