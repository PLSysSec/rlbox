#pragma once

#include <type_traits>

namespace rlbox {

#define RLBOX_ENABLE_IF(...) std::enable_if_t<__VA_ARGS__>* = nullptr

template<typename T>
using non_void_t = std::conditional_t<std::is_void_v<T>, int, T>;

template<typename T>
constexpr bool is_fundamental_or_enum_v =
  std::is_fundamental_v<T> || std::is_enum_v<T>;

template<typename T>
constexpr bool is_basic_type_v =
  std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> ||
  std::is_null_pointer_v<T>;

template<typename T>
using valid_return_t = std::decay_t<T>;
}