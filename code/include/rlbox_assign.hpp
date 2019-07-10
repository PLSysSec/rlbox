#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <cstring>

#include "rlbox_helpers.hpp"
#include "rlbox_types.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox::detail {

#define KEEP_ASSIGNMENT_FRIENDLY                                               \
  template<typename T_A_Lhs,                                                   \
           typename T_A_Rhs,                                                   \
           typename T_A_Sbx,                                                   \
           template<typename, typename>                                        \
           typename T_A_Lhs_wrap,                                              \
           template<typename, typename>                                        \
           typename T_A_Rhs_wrap>                                              \
  friend inline void detail::assign_wrapped_value_primitive(                   \
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
  friend inline void detail::assign_wrapped_value_nonclass(                    \
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
inline void assign_wrapped_value_nonclass(T_Lhs_wrap<T_Lhs, T_Sbx>& lhs,
                                          const T_Rhs_wrap<T_Rhs, T_Sbx>& rhs)
{
  using namespace std;
  using T_Lhs_El = remove_all_extents_t<T_Lhs>;
  using T_Rhs_El =
    remove_all_extents_t<typename T_Rhs_wrap<T_Rhs, T_Sbx>::T_ConvertedType>;

  static_assert(is_base_of_v<sandbox_wrapper_base, T_Lhs_wrap<T_Lhs, T_Sbx>>);
  static_assert(is_base_of_v<sandbox_wrapper_base, T_Rhs_wrap<T_Rhs, T_Sbx>>);
  static_assert(is_assignable_v<T_Lhs&, T_Rhs> || is_same_v<T_Lhs, T_Rhs>);

  // TODO: remaining optimization exists. If the swizzling is a noop, also
  // use a block copy

  if_constexpr_named(cond1, is_basic_type_v<T_Lhs>)
  {
    assign_wrapped_value_primitive(lhs, rhs);
  }
  else if_constexpr_named(cond2, detail::is_func_or_func_ptr<T_Lhs>)
  {
    // function pointers cannot be assigned this way, so just set it to null
    // tainted_volatile pointer types may be a numeric type
    if constexpr (is_pointer_v<decltype(lhs.data)>) {
      lhs.data = nullptr;
    } else {
      lhs.data = 0;
    }
  }
  else if_constexpr_named(cond3,
                          is_array_v<T_Lhs> &&
                            is_fundamental_or_enum_v<T_Lhs_El> &&
                            sizeof(T_Lhs_El) == sizeof(T_Rhs_El))
  {
    // Something like an int[5] and the machine models are identical
    // we can optimize this case by doing a block copy
    const void* src = remove_volatile_from_ptr_cast(&rhs.data);
    void* dest = remove_volatile_from_ptr_cast(&lhs.data);
    std::memcpy(dest, src, sizeof(T_Lhs));
  }
  else if_constexpr_named(cond4, is_array_v<T_Lhs> && is_basic_type_v<T_Lhs_El>)
  {
    // Something like an int[5], but the machine models are not identical
    // or something like void*[5] which may require pointer swizzling
    // in this scenario, use a simple for loop

    static_assert(std::extent_v<T_Lhs> == std::extent_v<T_Rhs>,
                  "Unexpected assignment of arrays of different sizes");

    for (size_t i = 0; i < std::extent_v<T_Lhs>; i++) {
      assign_wrapped_value_nonclass(lhs[i], rhs[i]);
    }
  }
  else
  {
    constexpr auto unknownCase = !(cond1 || cond2 || cond3 || cond4);
    rlbox_detail_static_fail_because(
      unknownCase, "Unexpected case for assign_wrapped_value_nonclass");
  }
}

template<typename T>
void assign_or_copy(T& lhs, T&& rhs)
{
  if constexpr (std::is_assignable_v<T&, T>) {
    lhs = rhs;
  } else {
    // Use memcpy as types like static arrays are not assignable with =
    auto dest = reinterpret_cast<void*>(&lhs);
    auto src = reinterpret_cast<const void*>(&rhs);
    std::memcpy(dest, src, sizeof(T));
  }
}

// specialization for array decays
template<typename T, RLBOX_ENABLE_IF(std::is_array_v<T>)>
void assign_or_copy(T& lhs, std::decay_t<T> rhs)
{
  // Use memcpy as types like static arrays are not assignable with =
  auto dest = reinterpret_cast<void*>(&lhs);
  auto src = reinterpret_cast<const void*>(rhs);
  std::memcpy(dest, src, sizeof(T));
}

}