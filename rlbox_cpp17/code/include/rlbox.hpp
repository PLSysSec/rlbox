#pragma once

#include <functional>
#include <numeric>

#include "rlbox_helpers.hpp"
using namespace std;

namespace rlbox {

template<typename T_to, typename T_from>
inline constexpr T_to convert_fundamental(const T_from& val)
{
  if_constexpr_named(cond1, !is_fundamental_v<T_from>)
  {
    rlbox_detail_static_fail(!cond1,
                             "Conversion source should be fundamental type");
  }
  else if_constexpr_named(cond2, !is_fundamental_v<T_to>)
  {
    rlbox_detail_static_fail(!cond2,
                             "Conversion target should be fundamental type");
  }
  else if_constexpr_named(
    cond3, is_floating_point_v<T_from> && is_floating_point_v<T_to>)
  {
    // language coerces different float types
    return val;
  }
  else if_constexpr_named(cond4,
                          is_pointer_v<T_from> || is_null_pointer_v<T_from>)
  {
    // pointers just get truncated or extended
    return static_cast<uintptr_t>(val);
  }
  else if_constexpr_named(
    cond5, is_integral_v<T_from> && is_signed_v<T_from> != is_signed_v<T_to>)
  {
    rlbox_detail_static_fail(
      !cond5, "Conversion should not go between signed and unsigned");
  }
  else if_constexpr_named(cond6, is_integral_v<T_from>)
  {

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
  }
  else
  {
    constexpr auto unknownCase =
      !(cond1 || cond2 || cond3 || cond4 || cond5 || cond6);
    rlbox_detail_static_fail(unknownCase, "Unexpected case for safe_convert");
  }
}

};
