#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <type_traits>

#include "rlbox_types.hpp"

namespace rlbox::detail {

namespace detail_rlbox_is_tainted {

  template<typename T, typename T_Sbx>
  std::true_type helper(tainted<T, T_Sbx>*);

  template<typename T>
  std::false_type helper(T*);
}

template<typename T>
constexpr bool rlbox_is_tainted_v = decltype(detail_rlbox_is_tainted::helper(
  std::declval<std::remove_cv_t<std::remove_reference_t<T>>*>()))::value;

namespace detail_rlbox_is_tainted_volatile {

  template<typename T, typename T_Sbx>
  std::true_type helper(tainted_volatile<T, T_Sbx>*);

  template<typename T>
  std::false_type helper(T*);
}

template<typename T>
constexpr bool rlbox_is_tainted_volatile_v =
  decltype(detail_rlbox_is_tainted_volatile::helper(
    std::declval<std::remove_cv_t<std::remove_reference_t<T>>*>()))::value;

namespace detail_rlbox_is_sandbox_callback {

  template<typename T, typename T_Sbx>
  std::true_type helper(sandbox_callback<T, T_Sbx>*);

  template<typename T>
  std::false_type helper(T*);
}

template<typename T>
constexpr bool rlbox_is_sandbox_callback_v =
  decltype(detail_rlbox_is_sandbox_callback::helper(
    std::declval<std::remove_cv_t<std::remove_reference_t<T>>*>()))::value;

template<typename T>
constexpr bool rlbox_is_tainted_or_vol_v =
  rlbox_is_tainted_v<T> || rlbox_is_tainted_volatile_v<T>;

template<typename T>
constexpr bool rlbox_is_wrapper_v =
  rlbox_is_tainted_v<T> || rlbox_is_tainted_volatile_v<T> ||
  rlbox_is_sandbox_callback_v<T>;

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