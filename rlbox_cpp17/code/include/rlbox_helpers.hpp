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
      throw std::runtime_error(Message)
#  else
#    define rlbox_detail_static_fail_because(CondExpr, Message) abort()
#  endif
#else
#  define rlbox_detail_static_fail_because(CondExpr, Message)                  \
    static_assert(!(CondExpr), Message)
#endif

#define if_constexpr_named(varName, ...)                                       \
  if constexpr (constexpr auto varName = __VA_ARGS__; varName)

/*
Make sure classes can access the private memmbers of tainted<T1> and
tainted_volatile. Ideally, this should be

template <typename U1>
friend class tainted<U1, T_Sandbox>;

But C++ doesn't seem to allow the above
*/
#define KEEP_CLASSES_FRIENDLY                                                  \
  template<typename U1, typename U2>                                           \
  friend class tainted;                                                        \
                                                                               \
  template<typename U1, typename U2>                                           \
  friend class tainted_volatile;

};

}
