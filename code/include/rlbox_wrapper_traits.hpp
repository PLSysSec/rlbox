#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <type_traits>

#include "rlbox_types.hpp"

namespace rlbox::detail {

#define rlbox_generate_wrapper_check(name)                                     \
  namespace detail_rlbox_is_##name                                             \
  {                                                                            \
                                                                               \
    template<typename T, typename T_Sbx>                                       \
    std::true_type helper(name<T, T_Sbx>*);                                    \
                                                                               \
    template<typename T>                                                       \
    std::false_type helper(T*);                                                \
  }                                                                            \
                                                                               \
  template<typename T>                                                         \
  constexpr bool rlbox_is_##name##_v =                                         \
    decltype(detail_rlbox_is_##name::helper(                                   \
      std::declval<std::remove_cv_t<std::remove_reference_t<T>>*>()))::value;

rlbox_generate_wrapper_check(tainted)
rlbox_generate_wrapper_check(tainted_volatile)
rlbox_generate_wrapper_check(sandbox_callback)
rlbox_generate_wrapper_check(sandbox_function)

#undef rlbox_generate_wrapper_check

template<typename T>
constexpr bool rlbox_is_tainted_or_vol_v =
  rlbox_is_tainted_v<T> || rlbox_is_tainted_volatile_v<T>;

template<typename T>
constexpr bool rlbox_is_wrapper_v =
  rlbox_is_tainted_v<T> || rlbox_is_tainted_volatile_v<T> ||
  rlbox_is_sandbox_callback_v<T> || rlbox_is_sandbox_function_v<T>;

namespace detail_rlbox_remove_wrapper {

  template<class TData>
  struct unwrapper
  {
    using type = TData;
  };

  template<typename T, typename T_Sbx>
  unwrapper<T> helper(tainted<T, T_Sbx>*);

  template<typename T, typename T_Sbx>
  unwrapper<T> helper(tainted_volatile<T, T_Sbx>*);

  template<typename T, typename T_Sbx>
  unwrapper<T> helper(sandbox_callback<T, T_Sbx>*);

  template<typename T, typename T_Sbx>
  unwrapper<T> helper(sandbox_function<T, T_Sbx>*);

  template<typename T>
  unwrapper<T> helper(T*);

  template<typename T>
  using rlbox_remove_wrapper_helper = decltype(
    helper(std::declval<std::remove_cv_t<std::remove_reference_t<T>>*>()));
}

template<typename T>
using rlbox_remove_wrapper_t =
  typename detail_rlbox_remove_wrapper::rlbox_remove_wrapper_helper<T>::type;

// https://stackoverflow.com/questions/34974844/check-if-a-type-is-from-a-particular-namespace
namespace detail_is_member_of_rlbox_detail {
  template<typename T, typename = void>
  struct is_member_of_rlbox_detail_helper : std::false_type
  {};

  template<typename T>
  struct is_member_of_rlbox_detail_helper<
    T,
    decltype(struct_is_member_of_rlbox_detail(std::declval<T>()))>
    : std::true_type
  {};
}

template<typename T>
void struct_is_member_of_rlbox_detail(T&&);

template<typename T>
constexpr auto is_member_of_rlbox_detail =
  detail_is_member_of_rlbox_detail::is_member_of_rlbox_detail_helper<T>::value;

}