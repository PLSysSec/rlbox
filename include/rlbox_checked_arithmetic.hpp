/**
 * @file rlbox_checked_arithmetic.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This file contains functions to perform arithmetic without overflows.
 */
#pragma once

// IWYU incorrectly reports this as unnecessary as the use of type_traits is in
// a templated class
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_error_handling.hpp"

namespace rlbox::detail {

/**
 * @brief Function that adds two unsigned values and checks for overflow
 * @tparam T is the unsigned type on which we are doing arithmetic
 * @param aLhs is the first operand
 * @param aRhs is the second operand
 * @param aErrorMsg is the error message to pass @ref
 * rlbox::detail::dynamic_check on failure
 * @return T is the result of the opreation
 */
template <typename T>
T checked_add(T aLhs, T aRhs, const char* aErrorMsg) {
  static_assert(std::is_unsigned_v<T>, "Expected unsigned type");

  T ret = aLhs + aRhs;
  bool has_overflow = ret < aLhs;
  dynamic_check(!has_overflow, aErrorMsg);

  return ret;
}

/**
 * @brief Function that multiplies two unsigned values and checks for overflow
 * @tparam T is the unsigned type on which we are doing arithmetic
 * @param aLhs is the first operand
 * @param aRhs is the second operand
 * @param aErrorMsg is the error message to pass @ref
 * rlbox::detail::dynamic_check on failure
 * @return T is the result of the opreation
 */
template <typename T>
T checked_multiply(T aLhs, T aRhs, const char* aErrorMsg) {
  static_assert(std::is_unsigned_v<T>, "Expected unsigned type");

  T ret = aLhs * aRhs;
  bool has_overflow = (aLhs != 0) && ((ret / aLhs) != aRhs);
  dynamic_check(!has_overflow, aErrorMsg);

  return ret;
}

}  // namespace rlbox::detail
