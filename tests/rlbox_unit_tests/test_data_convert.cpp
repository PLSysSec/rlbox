/**
 * @file test_data_convert.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that conversion between different types works as expected.
 */

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>

#include "test_include.hpp"

using rlbox::detail::convert_type_fundamental;

TEST_CASE("convert_type_fundamental for numerics operates correctly",
          "[convert]") {
  const int32_t irand_value = 5;
  const uint32_t urand_value = 5;

  {
    int64_t dest{0};
    convert_type_fundamental(&dest, irand_value);
    REQUIRE(dest == irand_value);
  }
  {
    int64_t dest{0};
    convert_type_fundamental(&dest, irand_value);
    REQUIRE(dest == irand_value);
  }
  {
    uint64_t dest{0};
    convert_type_fundamental(&dest, urand_value);
    REQUIRE(dest == urand_value);
  }
  {
    uint64_t dest{0};
    convert_type_fundamental(&dest, urand_value);
    REQUIRE(dest == urand_value);
  }
}

TEST_CASE(
    "convert_type_fundamental dynamic checks for numerics operate correctly",
    "[convert]") {
  uint64_t u32_max = std::numeric_limits<uint32_t>::max();
  int64_t i32_max = std::numeric_limits<int32_t>::max();
  int64_t i32_min = std::numeric_limits<int32_t>::min();

  {
    uint32_t dest{0};
    REQUIRE_THROWS(convert_type_fundamental(&dest, u32_max + 1));
  }
  {
    int32_t dest{0};
    REQUIRE_THROWS(convert_type_fundamental(&dest, i32_max + 1));
  }
  {
    int32_t dest{0};
    REQUIRE_THROWS(convert_type_fundamental(&dest, i32_min - 1));
  }
}

// TEST_CASE("convert_type_fundamental_or_array for arrays operates correctly",
//           "[convert]")
// {
//   const int32_t c_arr_1[4]{ 1, 2, 3, 4 }; // NOLINT
//   const std::array<int32_t, 4> std_arr_1{ { 1, 2, 3, 4 } };

//   const int64_t c_arr_2[4]{ 1, 2, 3, 4 }; // NOLINT
//   const std::array<int64_t, 4> std_arr_2{ { 1, 2, 3, 4 } };

//   {
//     std::array<int32_t, 4> dest{};
//     convert_type_fundamental_or_array(dest, c_arr_1);
//     REQUIRE(dest == std_arr_1);
//   }

//   {
//     int32_t dest[4]{}; // NOLINT
//     convert_type_fundamental_or_array(dest, std_arr_1);
//     REQUIRE(std::memcmp(&dest, &c_arr_1, sizeof(dest)) == 0);
//   }

//   {
//     std::array<int32_t, 4> dest{};
//     convert_type_fundamental_or_array(dest, c_arr_2);
//     REQUIRE(dest == std_arr_1);
//   }

//   {
//     int32_t dest[4]{}; // NOLINT
//     convert_type_fundamental_or_array(dest, std_arr_2);
//     REQUIRE(std::memcmp(&dest, &c_arr_1, sizeof(dest)) == 0);
//   }
// }

// TEST_CASE(
//   "convert_type_fundamental_or_array checks for arrays operate correctly",
//   "[convert]")
// {
//   const int64_t c_arr[4]{ 1, std::numeric_limits<int64_t>::max(), 3, 4 };
//   int32_t dest[4]{}; // NOLINT
//   REQUIRE_THROWS(convert_type_fundamental_or_array(dest, c_arr));
// }
