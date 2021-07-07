#include <memory>

#include "test_include.hpp"

using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("Test bool operators", "[operator]")
{
  tainted<bool, TestSandbox> t_T = true;
  tainted<bool, TestSandbox> t_F = false;
  const bool T = true;

  SECTION("tainted left") // NOLINT
  {
    REQUIRE(t_T.UNSAFE_unverified());

    REQUIRE_FALSE(t_F.UNSAFE_unverified());

    REQUIRE((!t_F).UNSAFE_unverified());

    tainted<bool, TestSandbox> and_false = t_F && T;
    REQUIRE_FALSE(and_false.UNSAFE_unverified());

    tainted<bool, TestSandbox> and_true = t_T && T;
    REQUIRE(and_true.UNSAFE_unverified());

    REQUIRE_FALSE((t_T && t_F).UNSAFE_unverified());
    REQUIRE((t_T || t_F).UNSAFE_unverified());

    // No r-value expressions for boolean operators as short circuiting behavior
    // changes
    REQUIRE_COMPILE_ERR(t_T && true);
    REQUIRE_COMPILE_ERR(t_T && tainted<bool, TestSandbox>(true));
  }

  SECTION("tainted right") // NOLINT
  {
    tainted<bool, TestSandbox> and_false = T && t_F;
    REQUIRE_FALSE(and_false.UNSAFE_unverified());

    tainted<bool, TestSandbox> and_true = T && t_T;
    REQUIRE(and_true.UNSAFE_unverified());

    // No r-value expressions for boolean operators as short circuiting behavior
    // changes
    REQUIRE_COMPILE_ERR(true && tainted<bool, TestSandbox>(true));
  }
}
