#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <type_traits>

#include "rlbox_types.hpp"

namespace rlbox::detail {

#define RLBOX_ENABLE_IF(...) std::enable_if_t<__VA_ARGS__>* = nullptr

template<typename T>
constexpr bool is_fundamental_or_enum_v =
  std::is_fundamental_v<T> || std::is_enum_v<T>;

template<typename T>
constexpr bool is_basic_type_v =
  std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> ||
  std::is_null_pointer_v<T>;

template<typename T>
using valid_return_t =
  std::conditional_t<std::is_function_v<T>, void*, std::decay_t<T>>;

template<typename T>
using valid_param_t = std::conditional_t<std::is_void_v<T>, void*, T>;

template<typename T>
using dereference_result_t =
  std::conditional_t<std::is_pointer_v<T>,
                     std::remove_pointer_t<T>,
                     std::remove_extent_t<T> // is_array
                     >;

template<typename T>
constexpr bool is_func_or_func_ptr =
  std::is_function_v<T> || std::is_function_v<std::remove_pointer_t<T>> ||
  std::is_member_function_pointer_v<T>;

template<typename T>
constexpr bool is_one_level_ptr_v =
  std::is_pointer_v<T> && !std::is_pointer_v<std::remove_pointer_t<T>>;

template<typename T_This, typename T_Target>
using add_const_if_this_const_t =
  std::conditional_t<std::is_const_v<std::remove_pointer_t<T_This>>,
                     std::add_const_t<T_Target>,
                     T_Target>;

namespace detail_rlbox_remove_wrapper {

  template<class TData>
  struct unwrapper
  {
    using type = TData;
  };

  template<typename T>
  unwrapper<T> unwrap_helper(sandbox_wrapper_base_of<T>);

  template<typename T, typename T_Enable = void>
  struct rlbox_remove_wrapper_helper;

  template<typename T>
  struct rlbox_remove_wrapper_helper<
    T,
    std::enable_if_t<std::is_base_of_v<sandbox_wrapper_base, T>>>
  {
    using ret = decltype(unwrap_helper(std::declval<T>()));
    using type = typename ret::type;
  };

  template<typename T>
  struct rlbox_remove_wrapper_helper<
    T,
    std::enable_if_t<!std::is_base_of_v<sandbox_wrapper_base, T>>>
  {
    using type = T;
  };
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

// remove all pointers/extent types
namespace remove_all_pointers_detail {
  template<typename T>
  struct remove_all_pointers
  {
    typedef T type;
  };

  template<typename T>
  struct remove_all_pointers<T*>
  {
    typedef typename remove_all_pointers<T>::type type;
  };
}

template<typename T>
using remove_all_pointers_t =
  typename remove_all_pointers_detail::remove_all_pointers<T>::type;

// remove all pointers/extent types
namespace base_type_detail {
  template<typename T>
  struct base_type
  {
    typedef T type;
  };

  template<typename T>
  struct base_type<T*>
  {
    typedef typename base_type<T>::type type;
  };

  template<typename T>
  struct base_type<T[]>
  {
    typedef typename base_type<T>::type type;
  };

  template<typename T, std::size_t N>
  struct base_type<T[N]>
  {
    typedef typename base_type<T>::type type;
  };
}

template<typename T>
using base_type_t = typename base_type_detail::base_type<T>::type;

// convert types
namespace convert_detail {
  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType,
           typename T_Enable = void>
  struct convert_base_types_t_helper;

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_same_v<int, T> && !std::is_const_v<T>>>
  {
    using type = T_IntType;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_same_v<long, T> && !std::is_const_v<T>>>
  {
    using type = T_LongType;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_same_v<long long, T> && !std::is_const_v<T>>>
  {
    using type = T_LongLongType;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_pointer_v<T> && !std::is_const_v<T>>>
  {
    using type = T_PointerType;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_unsigned_v<T> && !std::is_const_v<T>>>
  {
    using type = std::make_unsigned_t<
      typename convert_base_types_t_helper<std::make_signed_t<T>,
                                           T_IntType,
                                           T_LongType,
                                           T_LongLongType,
                                           T_PointerType>::type>;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<(std::is_same_v<bool, T> || std::is_same_v<void, T> ||
                      std::is_same_v<char, T> ||
                      std::is_enum_v<T>)&&!std::is_const_v<T>>>
  {
    using type = T;
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<
    T,
    T_IntType,
    T_LongType,
    T_LongLongType,
    T_PointerType,
    std::enable_if_t<std::is_array_v<T> && !std::is_const_v<T>>>
  {
    using type = typename convert_base_types_t_helper<
      std::remove_extent_t<T>,
      T_IntType,
      T_LongType,
      T_LongLongType,
      T_PointerType>::type[std::extent_v<T>];
  };

  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<T,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType,
                                     std::enable_if_t<std::is_const_v<T>>>
  {
    using type = std::add_const_t<
      typename convert_base_types_t_helper<std::remove_const_t<T>,
                                           T_IntType,
                                           T_LongType,
                                           T_LongLongType,
                                           T_PointerType>::type>;
  };
}

template<typename T,
         typename T_IntType,
         typename T_LongType,
         typename T_LongLongType,
         typename T_PointerType>
using convert_base_types_t =
  typename convert_detail::convert_base_types_t_helper<T,
                                                       T_IntType,
                                                       T_LongType,
                                                       T_LongLongType,
                                                       T_PointerType>::type;

}