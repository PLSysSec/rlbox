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
 * - converting between differently sized, differentely signed integer types
 * - converting between floating types
 * When converting between a bigger integer type to a smaller integer types, we
 * will add bounds checks
 * - `convert<uint64_t, uint32_t>(&dest, val)` is just a static cast
 * - `convert<uint32_t, uint64_t>(&dest, val)` introduces dynamic bounds checks
 */
template <typename TTo, typename TFrom>
inline void convert_type_fundamental(TTo* aTo, const TFrom& aFrom) {
  using std::is_same_v, std::is_integral_v, std::is_floating_point_v,
      std::is_enum_v, std::is_unsigned_v, std::is_signed_v, std::numeric_limits;

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

    /* Algorithm for safety checks across signed and unsigned.

    Note that comparisons across different sized ints are safe (due to integer
    promotion) as long as the signs of the integers are the same.

    if (TTo is unsigned) {
      if (TFrom is unsigned) {
        aFrom <= TTo.max();
      } else if (TFrom is signed) {
        aFrom >= 0
        make_unsigned(aFrom) <= TTo.max();
      }
    } else if (TTo is signed) {
      if (TFrom is unsigned) {
        aFrom <= make_unsigned(TTo.max());
      } else if (TFrom is signed) {
        aFrom >= TTo.min();
        aFrom <= TTo.max();
      }
    }
    */

    // Some branches don't use the param
    [[maybe_unused]] const char* err_msg =
        "Over/Underflow when converting between integer types";

    if constexpr (std::is_unsigned_v<TTo>) {
      if constexpr (std::is_unsigned_v<TFrom>) {
        dynamic_check(aFrom <= numeric_limits<TTo>::max(), err_msg);
      } else {
        static_assert(std::is_signed_v<TFrom>);
        dynamic_check(aFrom >= 0, err_msg);
        using TFromUnsigned = std::make_unsigned_t<TFrom>;
        dynamic_check(
            static_cast<TFromUnsigned>(aFrom) <= numeric_limits<TTo>::max(),
            err_msg);
      }
    } else {
      static_assert(std::is_signed_v<TTo>);
      if constexpr (std::is_unsigned_v<TFrom>) {
        using TToUnsigned = std::make_unsigned_t<TTo>;
        dynamic_check(
            aFrom <= static_cast<TToUnsigned>(numeric_limits<TTo>::max()),
            err_msg);
      } else {
        static_assert(std::is_signed_v<TFrom>);
        dynamic_check(aFrom >= numeric_limits<TTo>::min(), err_msg);
        dynamic_check(aFrom <= numeric_limits<TTo>::max(), err_msg);
      }
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
