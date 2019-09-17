#include <array>
#include <cstdint>
#include <cstring>
#include <limits>

#include "test_include.hpp"

struct Foo
{
  int a;
};

using rlbox::detail::convert_type_fundamental_or_array;

// NOLINTNEXTLINE
TEST_CASE("convert_type_fundamental_or_array for numerics operates correctly",
          "[convert]")
{
  const int32_t randValue = 5;

  {
    int64_t dest;
    convert_type_fundamental_or_array(dest, randValue);
    REQUIRE(dest == randValue);
  }

  {
    int64_t dest;
    convert_type_fundamental_or_array(dest, randValue);
    REQUIRE(dest == randValue);
  }
}

// NOLINTNEXTLINE
TEST_CASE("convert_type_fundamental_or_array compile time checks for "
          "numerics operate correctly",
          "[convert]")
{
  const int32_t randValue = 5;
  Foo a{ randValue };
  // does not support classes
  {
    Foo dest{};
    REQUIRE_COMPILE_ERR(convert_type_fundamental_or_array(dest, a));
  }
  {
    Foo dest{};
    REQUIRE_COMPILE_ERR(convert_type_fundamental_or_array(dest, randValue));
  }
  {
    int dest;
    REQUIRE_COMPILE_ERR(convert_type_fundamental_or_array(dest, a));
  }
}

// NOLINTNEXTLINE
TEST_CASE("convert_type_fundamental_or_array dynamic bounds checks for "
          "numerics operate correctly",
          "[convert]")
{
  uint64_t u32Max = std::numeric_limits<uint32_t>::max();
  const uint64_t randValue = 5;

  {
    uint32_t dest;
    convert_type_fundamental_or_array(dest, randValue);
    REQUIRE(dest == randValue);
  }

  {
    uint32_t dest;
    REQUIRE_THROWS(convert_type_fundamental_or_array(dest, u32Max + 1));
  }
}

// NOLINTNEXTLINE
TEST_CASE("convert_type_fundamental_or_array for arrays operates correctly",
          "[convert]")
{
  const int32_t c_arr_1[4]{ 1, 2, 3, 4 }; // NOLINT
  const std::array<int32_t, 4> std_arr_1{ { 1, 2, 3, 4 } };

  const int64_t c_arr_2[4]{ 1, 2, 3, 4 }; // NOLINT
  const std::array<int64_t, 4> std_arr_2{ { 1, 2, 3, 4 } };

  {
    std::array<int32_t, 4> dest{};
    convert_type_fundamental_or_array(dest, c_arr_1);
    REQUIRE(dest == std_arr_1);
  }

  {
    int32_t dest[4]{}; // NOLINT
    convert_type_fundamental_or_array(dest, std_arr_1);
    REQUIRE(std::memcmp(&dest, &c_arr_1, sizeof(dest)) == 0);
  }

  {
    std::array<int32_t, 4> dest{};
    convert_type_fundamental_or_array(dest, c_arr_2);
    REQUIRE(dest == std_arr_1);
  }

  {
    int32_t dest[4]{}; // NOLINT
    convert_type_fundamental_or_array(dest, std_arr_2);
    REQUIRE(std::memcmp(&dest, &c_arr_1, sizeof(dest)) == 0);
  }
}

TEST_CASE(
  "convert_type_fundamental_or_array checks for arrays operate correctly",
  "[convert]")
{
  // NOLINTNEXTLINE
  const int64_t c_arr[4]{ 1, std::numeric_limits<int64_t>::max(), 3, 4 };
  int32_t dest[4]{}; // NOLINT
  REQUIRE_THROWS(convert_type_fundamental_or_array(dest, c_arr));
}