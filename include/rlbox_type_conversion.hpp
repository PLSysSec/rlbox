/**
 * @file rlbox_type_conversion.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains utilities to convert data across different ABIS
 */
#pragma once

#include <limits>
#include <type_traits>

#include "rlbox_error_handling.hpp"
#include "rlbox_type_traits.hpp"

namespace rlbox::detail {

/**
 * @brief This function converts primitive values like integers, floats or enums
 * across different ABIs
 * @tparam TFrom is the source type
 * @tparam TTo is the target type
 * @param aFrom is the value to be converted
 * @details This can be used to convert primitive values such as:
 * - converting between different sized unsigned integer types
 * - converting between different sized signed integer types
 * - converting between floating types
 * When converting between a bigger integer type to a smaller integer types, we
 * will add bounds checks
 * - `convert<uint64_t, uint32_t>(val)` is just a static cast
 * - `convert<uint32_t, uint64_t>(val)` introduces dynamic bounds checks
 */
template <typename TTo, typename TFrom>
inline constexpr std::remove_cv_t<TTo> convert_type_fundamental(
    const TFrom& aFrom) {
  using namespace std;

  std::remove_cv_t<TTo> ret;

  rlbox_static_assert(is_fundamental_or_enum_v<TTo>,
                      "Conversion target should be fundamental or enum type");
  rlbox_static_assert(is_fundamental_or_enum_v<TFrom>,
                      "Conversion source should be fundamental or enum type");

  if constexpr (is_same_v<remove_cvref_t<TTo>, remove_cvref_t<TFrom>>) {
    ret = aFrom;
  } else if constexpr (is_enum_v<remove_cvref_t<TTo>>) {
    rlbox_static_assert(false_v<TTo>,
                        "ABI convertor: Trying to assign enums of different "
                        "types to each other");
  } else if constexpr (is_floating_point_v<remove_cvref_t<TTo>>) {
    rlbox_static_assert(is_floating_point_v<remove_cvref_t<TTo>> &&
                            is_floating_point_v<remove_cvref_t<TFrom>>,
                        "ABI convertor: Trying to convert across "
                        "floating/non-floating point types");
    // language already coerces different float types
    ret = static_cast<TTo>(aFrom);
  } else if constexpr (is_integral_v<remove_cvref_t<TTo>>) {
    rlbox_static_assert(
        is_integral_v<remove_cvref_t<TTo>> &&
            is_integral_v<remove_cvref_t<TFrom>>,
        "ABI convertor: Trying to convert across integer/non-integer types");

    rlbox_static_assert(is_signed_v<TTo> == is_signed_v<TFrom>,
                        "ABI convertor: Trying to convert across "
                        "signed/unsigned integer types");

    // Some branches don't use the param
    [[maybe_unused]] const char* err_msg =
        "Over/Underflow when converting between integer types";

    if constexpr (sizeof(TTo) >= sizeof(TFrom)) {
      // Eg: int64_t aFrom int32_t, uint64_t aFrom uint32_t
    } else if constexpr (is_unsigned_v<TTo>) {
      // Eg: uint32_t aFrom uint64_t
      dynamic_check(aFrom <= numeric_limits<TTo>::max(), err_msg);
    } else if constexpr (is_signed_v<TTo>) {
      // Eg: int32_t aFrom int64_t
      dynamic_check(aFrom >= numeric_limits<TTo>::min(), err_msg);
      dynamic_check(aFrom <= numeric_limits<TTo>::max(), err_msg);
    } else {
      rlbox_static_assert(false_v<TTo>, "Unhandled case");
    }
    ret = static_cast<TTo>(aFrom);
  } else {
    rlbox_static_assert(false_v<TTo>,
                        "Unexpected case for convert_type_fundamental");
  }

  return ret;
}

}  // namespace rlbox::detail
