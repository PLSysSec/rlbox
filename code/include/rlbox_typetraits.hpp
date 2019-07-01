#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <type_traits>

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
using valid_return_t = std::decay_t<T>;

template<typename T>
using dereference_result_t =
  std::conditional_t<std::is_pointer_v<T>,
                     std::remove_pointer_t<T>,
                     std::remove_extent_t<T> // is_array
                     >;

// convert types
namespace convert_detail {
  template<typename T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper;

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<void,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = void;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<bool,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = bool;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<int,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = T_IntType;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<unsigned int,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = std::make_unsigned_t<T_IntType>;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<long,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = T_LongType;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<unsigned long,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = std::make_unsigned_t<T_LongType>;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<long long,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = T_LongLongType;
  };

  template<typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<unsigned long long,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = std::make_unsigned_t<T_LongLongType>;
  };

  template<class T,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<T*,
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = T_PointerType;
  };

  template<class T,
           std::size_t N,
           typename T_IntType,
           typename T_LongType,
           typename T_LongLongType,
           typename T_PointerType>
  struct convert_base_types_t_helper<T[N],
                                     T_IntType,
                                     T_LongType,
                                     T_LongLongType,
                                     T_PointerType>
  {
    using type = typename convert_base_types_t_helper<T,
                                                      T_IntType,
                                                      T_LongType,
                                                      T_LongLongType,
                                                      T_PointerType>::type[N];
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