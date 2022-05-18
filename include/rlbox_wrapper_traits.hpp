/**
 * @file rlbox_wrapper_traits.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc utilities to reflect on rlbox specific types
 */

#pragma once

#include <array>
#include <stddef.h>
#include <type_traits>

#include "rlbox_error_handling.hpp"
// IWYU incorrectly reports this as unnecessary as the use of rlbox_type_traits
// is in a templated class
#include "rlbox_type_traits.hpp"  // IWYU pragma: keep

namespace rlbox::detail {

template <typename T, typename TSbx>
struct tainted_value_type {
  using type = T;
};

template <typename T, typename TSbx, size_t TN>
struct tainted_value_type<T[TN], TSbx> {
  using type = std::array<T, TN>;
};

template <typename T, typename TSbx>
struct tainted_value_type<T[], TSbx> {
  static_assert(
      rlbox::detail::false_v<T>,
      "Dynamic arrays are currently unsupported. " RLBOX_FILE_BUG_MESSAGE);
};

/**
 * @brief RLBox provides various tainted wrappers that are used to mark any data
 * returned by the sandbox. For example, the function call to sandboxed code
 * that returns an `T` would return a `tainted<T>`. To ensure T, can be easily
 * wrapped we need to convert T to simple value types that we can pass around
 * easily. This trait implements these conversions
 * @details There conversions currently performed are
 * - Static array types (int[3]) are converted to std::array<int, 3>
 * - Other types are unchanged
 */
template <typename T, typename TSbx>
using tainted_value_type_t = typename tainted_value_type<T, TSbx>::type;

}  // namespace rlbox::detail