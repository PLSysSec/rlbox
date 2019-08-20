#include <cstdint>
#include <memory>
#include <type_traits>

#include "test_include.hpp"

using rlbox::tainted;
using rlbox::tainted_boolean_hint;

// NOLINTNEXTLINE
TEST_CASE("Test comparisons to nullptr", "[tainted_compare]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
  REQUIRE(ptr != nullptr);
  REQUIRE(!(ptr == nullptr));
  REQUIRE(!!ptr);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("Test disallowed comparisons to tainted", "[tainted_compare]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
  auto ptr2 = sandbox.malloc_in_sandbox<uint32_t>();
  REQUIRE_COMPILE_ERR(ptr < ptr2);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("Test comparisons to tainted", "[tainted_compare]")
{
  const uint32_t testVal = 3;
  tainted<uint32_t, TestSandbox> a = testVal;

  SECTION("Tainted Untainted compare") // NOLINT
  {
    auto result = a == testVal;
    REQUIRE(std::is_same_v<decltype(result), tainted<bool, TestSandbox>>);
    REQUIRE(result.UNSAFE_unverified());
    REQUIRE(!((!result).UNSAFE_unverified()));

    auto result2 = a != testVal;
    REQUIRE(std::is_same_v<decltype(result2), tainted<bool, TestSandbox>>);
    REQUIRE(!(result2.UNSAFE_unverified()));
    REQUIRE((!result2).UNSAFE_unverified());
  }

  SECTION("Untainted Tainted compare") // NOLINT
  {
    auto result = testVal == a;
    REQUIRE(std::is_same_v<decltype(result), tainted<bool, TestSandbox>>);
    REQUIRE(result.UNSAFE_unverified());
    REQUIRE(!((!result).UNSAFE_unverified()));

    auto result2 = testVal != a;
    REQUIRE(std::is_same_v<decltype(result2), tainted<bool, TestSandbox>>);
    REQUIRE(!(result2.UNSAFE_unverified()));
    REQUIRE((!result2).UNSAFE_unverified());
  }

  SECTION("Tainted Tainted compare") // NOLINT
  {
    auto result = a == a; // NOLINT(misc-redundant-expression)
    REQUIRE(std::is_same_v<decltype(result), tainted<bool, TestSandbox>>);
    REQUIRE(result.UNSAFE_unverified());
    REQUIRE(!((!result).UNSAFE_unverified()));

    auto result2 = a != a; // NOLINT(misc-redundant-expression)
    REQUIRE(std::is_same_v<decltype(result2), tainted<bool, TestSandbox>>);
    REQUIRE(!(result2.UNSAFE_unverified()));
    REQUIRE((!result2).UNSAFE_unverified());
  }
}

// NOLINTNEXTLINE
TEST_CASE("Test comparisons to tainted_volatile", "[tainted_compare]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const uint32_t testVal = 3;
  tainted<uint32_t, TestSandbox> t = testVal;
  auto t_ptr = sandbox.malloc_in_sandbox<uint32_t>();
  *t_ptr = testVal;

  SECTION("tainted with tainted_volatile") // NOLINT
  {
    auto result1 = t == (*t_ptr);
    auto result2 = t != (*t_ptr);
    REQUIRE(std::is_same_v<decltype(result1), tainted_boolean_hint>);
    REQUIRE(std::is_same_v<decltype(result2), tainted_boolean_hint>);
    REQUIRE(result1.unverified_safe_because("Testing"));
    REQUIRE(!result2.unverified_safe_because("Testing"));
  }
  SECTION("tainted_volatile with tainted") // NOLINT
  {
    auto result1 = (*t_ptr) == t;
    auto result2 = (*t_ptr) != t;
    REQUIRE(std::is_same_v<decltype(result1), tainted_boolean_hint>);
    REQUIRE(std::is_same_v<decltype(result2), tainted_boolean_hint>);
    REQUIRE(result1.unverified_safe_because("Testing"));
    REQUIRE(!result2.unverified_safe_because("Testing"));
  }
  SECTION("tainted_volatile with tainted_volatile") // NOLINT
  {
    auto result1 = (*t_ptr) == (*t_ptr); // NOLINT(misc-redundant-expression)
    auto result2 = (*t_ptr) != (*t_ptr); // NOLINT(misc-redundant-expression)
    REQUIRE(std::is_same_v<decltype(result1), tainted_boolean_hint>);
    REQUIRE(std::is_same_v<decltype(result2), tainted_boolean_hint>);
    REQUIRE(result1.unverified_safe_because("Testing"));
    REQUIRE(!result2.unverified_safe_because("Testing"));
  }
  SECTION("tainted_volatile with unwrapped") // NOLINT
  {
    auto result1 = (*t_ptr) == testVal;
    auto result2 = (*t_ptr) != testVal;
    REQUIRE(std::is_same_v<decltype(result1), tainted_boolean_hint>);
    REQUIRE(std::is_same_v<decltype(result2), tainted_boolean_hint>);
    REQUIRE(result1.unverified_safe_because("Testing"));
    REQUIRE(!result2.unverified_safe_because("Testing"));
  }
  SECTION("unwrapped with tainted_volatile") // NOLINT
  {
    auto result1 = testVal == (*t_ptr);
    auto result2 = testVal != (*t_ptr);
    REQUIRE(std::is_same_v<decltype(result1), tainted_boolean_hint>);
    REQUIRE(std::is_same_v<decltype(result2), tainted_boolean_hint>);
    REQUIRE(result1.unverified_safe_because("Testing"));
    REQUIRE(!result2.unverified_safe_because("Testing"));
  }

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("Test other comparison operators", "[tainted_compare]")
{
  tainted<uint32_t, TestSandbox> a = 1;
  auto ret = a > static_cast<uint32_t>(0);
  REQUIRE(ret.UNSAFE_unverified());
}