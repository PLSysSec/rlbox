#pragma once

#include "rlbox_helpers.hpp"
#include "rlbox_types.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

#define KEEP_ASSIGNMENT_FRIENDLY                                               \
  template<typename T_A_Lhs,                                                   \
           typename T_A_Rhs,                                                   \
           typename T_A_Sbx,                                                   \
           template<typename, typename>                                        \
           typename T_A_Lhs_wrap,                                              \
           template<typename, typename>                                        \
           typename T_A_Rhs_wrap>                                              \
  friend inline void assign_wrapped_value_primitive(                           \
    T_A_Lhs_wrap<T_A_Lhs, T_A_Sbx>& lhs,                                       \
    const T_A_Rhs_wrap<T_A_Rhs, T_A_Sbx>& rhs);                                \
                                                                               \
  template<typename T_A_Lhs,                                                   \
           typename T_A_Rhs,                                                   \
           typename T_A_Sbx,                                                   \
           template<typename, typename>                                        \
           typename T_A_Lhs_wrap,                                              \
           template<typename, typename>                                        \
           typename T_A_Rhs_wrap>                                              \
  friend inline void assign_wrapped_value(                                     \
    T_A_Lhs_wrap<T_A_Lhs, T_A_Sbx>& lhs,                                       \
    const T_A_Rhs_wrap<T_A_Rhs, T_A_Sbx>& rhs);

template<typename T_Lhs,
         typename T_Rhs,
         typename T_Sbx,
         template<typename, typename>
         typename T_Lhs_wrap,
         template<typename, typename>
         typename T_Rhs_wrap>
inline void assign_wrapped_value_primitive(T_Lhs_wrap<T_Lhs, T_Sbx>& lhs,
                                           const T_Rhs_wrap<T_Rhs, T_Sbx>& rhs)
{
  static_assert(is_basic_type_v<T_Lhs>);
  static_assert(is_basic_type_v<T_Rhs>);

  if_constexpr_named(
    cond1, std::is_base_of_v<tainted<T_Lhs, T_Sbx>, T_Lhs_wrap<T_Lhs, T_Sbx>>)
  {
    lhs.data = rhs.get_raw_value();
  }
  else if_constexpr_named(
    cond2,
    std::is_base_of_v<tainted_volatile<T_Lhs, T_Sbx>, T_Lhs_wrap<T_Lhs, T_Sbx>>)
  {
    lhs.data = rhs.get_raw_sandbox_value();
  }
  else
  {
    constexpr auto unknownCase = !(cond1 || cond2);
    rlbox_detail_static_fail_because(
      unknownCase, "Unexpected case for assign_wrapped_value_primitive");
  }
}

template<typename T_Lhs,
         typename T_Rhs,
         typename T_Sbx,
         template<typename, typename>
         typename T_Lhs_wrap,
         template<typename, typename>
         typename T_Rhs_wrap>
inline void assign_wrapped_value(T_Lhs_wrap<T_Lhs, T_Sbx>& lhs,
                                 const T_Rhs_wrap<T_Rhs, T_Sbx>& rhs)
{
  using namespace std;
  using T_Lhs_El = remove_all_extents_t<T_Lhs>;
  using T_Rhs_El = remove_all_extents_t<T_Rhs>;

  static_assert(is_base_of_v<sandbox_wrapper_base, T_Lhs_wrap<T_Lhs, T_Sbx>>);
  static_assert(is_base_of_v<sandbox_wrapper_base, T_Rhs_wrap<T_Rhs, T_Sbx>>);
  static_assert(is_assignable_v<T_Lhs&, T_Rhs>);

  // TODO: remaining optimization exists. If the swizzling is a noop, also
  // use a block copy

  if_constexpr_named(cond1, is_basic_type_v<T_Lhs>)
  {
    assign_wrapped_value_primitive(lhs, rhs);
  }
  else if_constexpr_named(cond2,
                          is_array_v<T_Lhs> &&
                            is_fundamental_or_enum_v<T_Lhs_El> &&
                            sizeof(T_Lhs_El) == sizeof(T_Rhs_El))
  {
    // Something like an int[5] and the machine models are identical
    // we can optimize this case by doing a block copy
    memcpy(lhs.data, rhs.data, sizeof(T_Lhs));
  }
  else if_constexpr_named(cond3, is_array_v<T_Lhs> && is_basic_type_v<T_Lhs_El>)
  {
    // Something like an int[5], but the machine models are identical
    // or something like void*[5] which may require pointer swizzling
    // in this scenario, use a simple for loop

    // Reduce to a one dimensional array and then copy
    auto lhs_arr = reinterpret_cast<T_Lhs_El*>(lhs);
    auto rhs_arr = reinterpret_cast<T_Rhs_El*>(rhs);

    for (size_t i = 0; i < sizeof(T_Lhs) / sizeof(T_Lhs_El); i++) {
      assign_wrapped_value(lhs_arr[i], rhs_arr[i]);
    }
  }
  else
  {
    constexpr auto unknownCase = !(cond1 || cond2 || cond3);
    rlbox_detail_static_fail_because(
      unknownCase, "Unexpected case for assign_wrapped_value");
  }
}

}