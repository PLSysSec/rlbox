#include <cstdint>
#include <limits>
#include <type_traits>

#include "test_include.hpp"

struct Foo
{
  int a;
};

using rlbox::detail::adjust_type_size;

// NOLINTNEXTLINE
TEST_CASE("Type Convert function operates correctly", "[convert]")
{
  const int32_t randValue = 5;

  {
    auto a = adjust_type_size<int64_t, int32_t>(randValue);
    REQUIRE(std::is_same_v<decltype(a), int64_t>);
    REQUIRE(a == randValue);
  }

  {
    auto a = adjust_type_size<int64_t, const int32_t>(randValue);
    REQUIRE(std::is_same_v<decltype(a), int64_t>);
    REQUIRE(a == randValue);
  }

  {
    auto a = adjust_type_size<const int64_t, int32_t>(randValue);
    REQUIRE(std::is_same_v<decltype(a), int64_t>);
    REQUIRE(a == randValue);
  }

  {
    auto a = adjust_type_size<const int64_t, const int32_t>(randValue);
    REQUIRE(std::is_same_v<decltype(a), int64_t>);
    REQUIRE(a == randValue);
  }
}

// NOLINTNEXTLINE
TEST_CASE("Type Convert function compile time checks operate correctly",
          "[convert]")
{
  const int32_t randValue = 5;
  REQUIRE_COMPILE_ERR(adjust_type_size<uint64_t, int32_t>(randValue)); // NOLINT
  REQUIRE_COMPILE_ERR(adjust_type_size<int64_t, uint32_t>(randValue)); // NOLINT

  Foo a{ randValue };
  REQUIRE_COMPILE_ERR(adjust_type_size<Foo, Foo>(a));         // NOLINT
  REQUIRE_COMPILE_ERR(adjust_type_size<Foo, int>(randValue)); // NOLINT
  REQUIRE_COMPILE_ERR(adjust_type_size<int, Foo>(a));         // NOLINT
}

// NOLINTNEXTLINE
TEST_CASE("Type Convert function dynamic bounds checks operate correctly",
          "[convert]")
{
  uint64_t u32Max = std::numeric_limits<uint32_t>::max();
  REQUIRE(adjust_type_size<uint32_t, uint64_t>(5) == 5);            // NOLINT
  REQUIRE_THROWS(adjust_type_size<uint32_t, uint64_t>(u32Max + 1)); // NOLINT
}
