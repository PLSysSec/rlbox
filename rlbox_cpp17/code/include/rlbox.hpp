#pragma once

#include <functional>
#include <numeric>

#include "rlbox_helpers.hpp"
using namespace std;

namespace rlbox {

template<typename T_to, typename T_from>
inline constexpr T_to convert_fundamental(const T_from& val)
{

  rlbox_detail_static_check(is_fundamental_v<T_from>,
                            "Conversion source should be fundamental type");
  rlbox_detail_static_check(is_fundamental_v<T_to>,
                            "Conversion target should be fundamental type");

  if constexpr (constexpr auto cond1 =
                  is_floating_point_v<T_from> && is_floating_point_v<T_to>;
                cond1) {
    // language coerces different float types
    return val;
  } else if constexpr (constexpr auto cond2 =
                         is_pointer_v<T_from> || is_null_pointer_v<T_from>;
                       cond2) {
    // pointers just get truncated or extended
    return static_cast<uintptr_t>(val);
  } else if constexpr (constexpr auto cond3 = is_integral_v<T_from>; cond3) {

    rlbox_detail_static_check(
      is_signed_v<T_from> == is_signed_v<T_to>,
      "Conversion should not go between signed and unsigned");

    if constexpr (sizeof(T_to) >= sizeof(T_from)) {
      return val;
    } else {
      // Only check upper for unsigned
      rlbox::detail::dynamic_check(val <= numeric_limits<T_to>::max(),
                                   "Overflow/underflow when converting value "
                                   "to a type with smaller range");
      if constexpr (is_signed_v<T_from>) {
        rlbox::detail::dynamic_check(val >= numeric_limits<T_to>::min(),
                                     "Overflow/underflow when converting value "
                                     "to a type with smaller range");
      }
      return static_cast<T_to>(val);
    }
  } else {
    rlbox_detail_static_check(cond1 || cond2 || cond3,
                              "Unexpected case for safe_convert");
  }
}

};
