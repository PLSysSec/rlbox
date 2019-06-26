#pragma once

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
    #ifdef RLBOX_USE_EXCEPTIONS
      throw std::runtime_error(msg);
    #else
      std::cerr << msg << std::endl;
      abort();
    #endif
  }
  // clang-format on
}

// clang-format off
#ifdef RLBOX_NO_COMPILE_CHECKS

  #if defined(RLBOX_USE_EXCEPTIONS)
    #define rlbox_detail_static_fail(CondExpr, Message)                        \
      if (!(CondExpr)) throw std::runtime_error(Message)
  #else
    #define rlbox_detail_static_fail(CondExpr, Message)                        \
      if (!(CondExpr)) abort()
  #endif

#else

  #define rlbox_detail_static_fail(CondExpr, Message)                          \
    static_assert(CondExpr, Message)

#endif
// clang-format on

#define if_constexpr_named(var, cond)                                          \
  if constexpr (constexpr auto var = cond; cond)

};

}
