/**
 * @file rlbox_data_conversion.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains utilities to convert data across different ABIS
 */
#pragma once

#include <limits>
#include <type_traits>

#include "rlbox_error_handling.hpp"
#include "rlbox_type_traits.hpp"
#include "rlbox_types.hpp"

namespace rlbox::detail {

/**
 * @brief This function converts primitive values like integers, floats or enums
 * across different ABIs
 * @tparam TFrom is the source type
 * @tparam TTo is the target type
 * @param aFrom is the value to be converted
 * @param aTo is pointer to the destination which will hold the converted value
 * @details This can be used to convert primitive values such as:
 * - converting between different sized unsigned integer types
 * - converting between different sized signed integer types
 * - converting between floating types
 * When converting between a bigger integer type to a smaller integer types, we
 * will add bounds checks
 * - `convert<uint64_t, uint32_t>(&dest, val)` is just a static cast
 * - `convert<uint32_t, uint64_t>(&dest, val)` introduces dynamic bounds checks
 */
template <typename TTo, typename TFrom>
inline void convert_type_fundamental(TTo* aTo, const TFrom& aFrom) {
  /// \todo Replace with specific imports and fix iwyu file
  using namespace std;

  rlbox_static_assert(is_fundamental_or_enum_v<TTo>,
                      "Conversion target should be fundamental or enum type");
  rlbox_static_assert(is_fundamental_or_enum_v<TFrom>,
                      "Conversion source should be fundamental or enum type");

  if constexpr (is_same_v<remove_cvref_t<TTo>, remove_cvref_t<TFrom>>) {
    *aTo = aFrom;
  } else if constexpr (is_enum_v<remove_cvref_t<TTo>>) {
    rlbox_static_fail(TTo,
                      "ABI convertor: Trying to assign enums of different "
                      "types to each other");
  } else if constexpr (is_floating_point_v<remove_cvref_t<TTo>>) {
    rlbox_static_assert(is_floating_point_v<remove_cvref_t<TTo>> &&
                            is_floating_point_v<remove_cvref_t<TFrom>>,
                        "ABI convertor: Trying to convert across "
                        "floating/non-floating point types");
    // language already coerces different float types
    *aTo = static_cast<TTo>(aFrom);
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
      rlbox_static_fail(TTo, "Unhandled case");
    }
    *aTo = static_cast<TTo>(aFrom);
  } else {
    rlbox_static_fail(TTo, "Unexpected case for convert_type_fundamental");
  }
}

enum class rlbox_convert_direction { TO_SANDBOX, TO_APPLICATION };

enum class rlbox_convert_style { EXAMPLE, SANDBOX };

template <typename TTo, typename TFrom, typename TSbx,
          rlbox_convert_style TContext, rlbox_convert_direction TDirection>
inline void convert_type_pointer(TTo* aTo, const TFrom& aFrom,
                                 rlbox_sandbox<TSbx>* aSandbox,
                                 const void* aExampleUnsandboxedPtr) {
  if constexpr (TContext == rlbox_convert_style::SANDBOX) {
    rlbox::detail::dynamic_check(aSandbox != nullptr,
                                 "Conversion with a null sandbox ptr");
    if constexpr (TDirection == rlbox_convert_direction::TO_SANDBOX) {
      *aTo = aSandbox->get_sandboxed_pointer(aFrom);
    } else {
      *aTo = aSandbox->template get_unsandboxed_pointer<TTo>(aFrom);
    }
  } else {
    if constexpr (TDirection == rlbox_convert_direction::TO_SANDBOX) {
      *aTo = rlbox_sandbox<TSbx>::get_sandboxed_pointer_with_example(
          aFrom, aExampleUnsandboxedPtr);
    } else {
      *aTo = rlbox_sandbox<TSbx>::template get_unsandboxed_pointer_with_example<
          TTo>(aFrom, aExampleUnsandboxedPtr);
    }
  }
}

}  // namespace rlbox::detail
