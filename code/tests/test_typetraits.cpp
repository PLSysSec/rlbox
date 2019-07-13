#include <array>
#include <cstddef>
#include <type_traits>

#include "test_include.hpp"

using rlbox::detail::all_extents_same;
using rlbox::detail::is_c_or_std_array_v;
using rlbox::detail::std_array_to_c_arr_t;

// NOLINTNEXTLINE
TEST_CASE("all_extents_same", "[type_traits]")
{
  REQUIRE(all_extents_same<int[4], long[4]>);           // NOLINT
  REQUIRE(!all_extents_same<int[4], long[3]>);          // NOLINT
  REQUIRE(all_extents_same<int[4][3], long[4][3]>);     // NOLINT
  REQUIRE(!all_extents_same<int[3][4], long[4][3]>);    // NOLINT
  REQUIRE(!all_extents_same<int[4][3], long[4][3][2]>); // NOLINT
  REQUIRE(!all_extents_same<int[4][3][2], long[4][3]>); // NOLINT
}

template<typename T, size_t N>
struct W
{};

// NOLINTNEXTLINE
TEST_CASE("is_c_or_std_array_v", "[type_traits]")
{
  REQUIRE(is_c_or_std_array_v<int[4]>);                // NOLINT
  REQUIRE(is_c_or_std_array_v<std::array<int, 4>>);    // NOLINT
  REQUIRE(is_c_or_std_array_v<std::array<int[4], 4>>); // NOLINT
  REQUIRE(is_c_or_std_array_v<std::array<int, 4>[4]>); // NOLINT
  REQUIRE(!is_c_or_std_array_v<int>);                  // NOLINT
  REQUIRE(!is_c_or_std_array_v<W<int, 4>>);            // NOLINT
  REQUIRE(is_c_or_std_array_v<W<int, 4>[4]>);          // NOLINT
}

// NOLINTNEXTLINE
TEST_CASE("std_array_to_c_arr_t", "[type_traits]")
{
  REQUIRE(
    std::is_same_v<std_array_to_c_arr_t<std::array<int, 4>>, int[4]>); // NOLINT
  REQUIRE(std::is_same_v<std_array_to_c_arr_t<int>, int>);             // NOLINT
  REQUIRE(std::is_same_v<std_array_to_c_arr_t<int[4]>, int[4]>);       // NOLINT
}