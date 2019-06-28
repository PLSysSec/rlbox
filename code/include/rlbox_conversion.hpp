#pragma once

#include <functional>
#include <numeric>

#include "rlbox_helpers.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

template<typename T_To, typename T_From>
inline constexpr T_To adjust_type_size(const T_From& val)
{
  using namespace std;

  if_constexpr_named(cond1, !is_basic_type_v<T_From>)
  {
    rlbox_detail_static_fail_because(
      cond1, "Conversion source should be fundamental, enum or pointer type");
  }
  else if_constexpr_named(cond2, !is_basic_type_v<T_To>)
  {
    rlbox_detail_static_fail_because(
      cond2, "Conversion target should be fundamental, enum or pointer type");
  }
  else if_constexpr_named(
    cond3, is_floating_point_v<T_From> && is_floating_point_v<T_To>)
  {
    // language coerces different float types
    return val;
  }
  else if_constexpr_named(cond4,
                          is_pointer_v<T_From> || is_null_pointer_v<T_From>)
  {
    // pointers just get truncated or extended
    return static_cast<uintptr_t>(val);
  }
  else if_constexpr_named(
    cond5, is_integral_v<T_From> && is_signed_v<T_From> != is_signed_v<T_To>)
  {
    rlbox_detail_static_fail_because(
      cond5, "Conversion should not go between signed and unsigned");
  }
  else if_constexpr_named(cond6, is_integral_v<T_From>)
  {
    if constexpr (sizeof(T_To) >= sizeof(T_From)) {
      return val;
    } else {
      // Only check upper for unsigned
      rlbox::detail::dynamic_check(val <= numeric_limits<T_To>::max(),
                                   "Overflow/underflow when converting value "
                                   "to a type with smaller range");
      if constexpr (is_signed_v<T_From>) {
        rlbox::detail::dynamic_check(val >= numeric_limits<T_To>::min(),
                                     "Overflow/underflow when converting value "
                                     "to a type with smaller range");
      }
      return static_cast<T_To>(val);
    }
  }
  else
  {
    constexpr auto unknownCase =
      !(cond1 || cond2 || cond3 || cond4 || cond5 || cond6);
    rlbox_detail_static_fail_because(unknownCase,
                                     "Unexpected case for adjust_type_size");
  }
}

};
