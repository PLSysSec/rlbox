#include "test_include.hpp"

struct Foo
{
  int a;
};

// NOLINTNEXTLINE
TEST_CASE("Type Convert function operates correctly", "[convert]")
{
  const int32_t randValue = 5;
  auto a = rlbox::convert_fundamental<int64_t, int32_t>(randValue);
  REQUIRE(std::is_same_v<decltype(a), int64_t>);
  REQUIRE(a == randValue);
}

// NOLINTNEXTLINE
TEST_CASE("Type Convert function compile time checks operate correctly",
          "[convert]")
{
  // Have configured rlbox to convert compile time checks to runtime throws
  const int32_t randValue = 5;
  REQUIRE_THROWS(
    rlbox::convert_fundamental<uint64_t, int32_t>(randValue)); // NOLINT
  REQUIRE_THROWS(
    rlbox::convert_fundamental<int64_t, uint32_t>(randValue)); // NOLINT
  Foo a{ randValue };
  REQUIRE_THROWS(rlbox::convert_fundamental<Foo, Foo>(a));         // NOLINT
  REQUIRE_THROWS(rlbox::convert_fundamental<Foo, int>(randValue)); // NOLINT
  REQUIRE_THROWS(rlbox::convert_fundamental<int, Foo>(a));         // NOLINT
}

// NOLINTNEXTLINE
TEST_CASE("Type Convert function dynamic bounds checks operate correctly",
          "[convert]")
{
  uint64_t u32Max = std::numeric_limits<uint32_t>::max();
  REQUIRE(rlbox::convert_fundamental<uint32_t, uint64_t>(5) == 5); // NOLINT
  REQUIRE_THROWS(
    rlbox::convert_fundamental<uint32_t, uint64_t>(u32Max + 1)); // NOLINT
}
