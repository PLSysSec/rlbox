#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <iostream>
#include <stdexcept>
#include <stdlib.h>

namespace rlbox {
namespace detail {
  const int CompileErrorCode = 42;

  inline void dynamic_check(bool check, const char* const msg)
  {
    // clang-format off
  if (!check) {
    #if __cpp_exceptions && defined(RLBOX_USE_EXCEPTIONS)
      throw std::runtime_error(msg);
    #else
      std::cerr << msg << std::endl;
      abort();
    #endif
  }
    // clang-format on
  }

#ifdef RLBOX_NO_COMPILE_CHECKS
#  if __cpp_exceptions && defined(RLBOX_USE_EXCEPTIONS)
#    define rlbox_detail_static_fail_because(CondExpr, Message)                \
      ((void)(CondExpr)), throw std::runtime_error(Message)
#  else
#    define rlbox_detail_static_fail_because(CondExpr, Message) abort()
#  endif
#else
#  define rlbox_detail_static_fail_because(CondExpr, Message)                  \
    static_assert(!(CondExpr), Message)
#endif

#define if_constexpr_named(varName, ...)                                       \
  if constexpr (constexpr auto varName = __VA_ARGS__; varName)

  template<typename... TArgs>
  void printTypes()
  {
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
    std::cout << __PRETTY_FUNCTION__ << std::endl; // NOLINT
#elif defined(_MSC_VER)
    std::cout << __FUNCSIG__ << std::endl; // NOLINT
#else
    std::cout << "Unsupported" << std::endl;
#endif
  }

#define rlbox_detail_forward_binop_to_base(opSymbol, ...)                      \
  template<typename T_Rhs>                                                     \
  inline auto operator opSymbol(T_Rhs rhs)                                     \
  {                                                                            \
    auto b = static_cast<__VA_ARGS__*>(this);                                  \
    return (*b)opSymbol rhs;                                                   \
  }

#define rlbox_detail_for_both(a, b, ...) a __VA_ARGS__ b __VA_ARGS__

  template<typename T>
  inline auto remove_volatile_from_ptr_cast(T* ptr)
  {
    using T_Result = std::add_pointer_t<std::remove_volatile_t<T>>;
    return const_cast<T_Result>(ptr);
  }

/*
Make sure classes can access the private memmbers of tainted<T1> and
tainted_volatile. Ideally, this should be

template <typename U1>
friend class tainted<U1, T_Sandbox>;

But C++ doesn't seem to allow the above
*/
#define KEEP_CLASSES_FRIENDLY                                                  \
  template<template<typename, typename> typename U1, typename U2, typename U3> \
  friend class tainted_base_impl;                                              \
                                                                               \
  template<typename U1, typename U2>                                           \
  friend class tainted;                                                        \
                                                                               \
  template<typename U1, typename U2>                                           \
  friend class tainted_volatile;                                               \
                                                                               \
  template<typename U1>                                                        \
  friend class RLBoxSandbox;

}

}
