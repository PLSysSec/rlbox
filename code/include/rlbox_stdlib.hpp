#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <cstring>
#include <type_traits>

#include "rlbox_types.hpp"
#include "rlbox_unwrap.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {
#define KEEP_CAST_FRIENDLY                                                     \
  template<typename T_C_Lhs,                                                   \
           typename T_C_Rhs,                                                   \
           typename T_C_Sbx,                                                   \
           template<typename, typename>                                        \
           typename T_C_Wrap>                                                  \
  friend inline tainted<T_C_Lhs, T_C_Sbx> sandbox_reinterpret_cast(            \
    const T_C_Wrap<T_C_Rhs, T_C_Sbx>& rhs) noexcept;                           \
                                                                               \
  template<typename T_C_Lhs,                                                   \
           typename T_C_Rhs,                                                   \
           typename T_C_Sbx,                                                   \
           template<typename, typename>                                        \
           typename T_C_Wrap>                                                  \
  friend inline tainted<T_C_Lhs, T_C_Sbx> sandbox_const_cast(                  \
    const T_C_Wrap<T_C_Rhs, T_C_Sbx>& rhs) noexcept;

template<typename T_Lhs,
         typename T_Rhs,
         typename T_Sbx,
         template<typename, typename>
         typename T_Wrap>
inline tainted<T_Lhs, T_Sbx> sandbox_reinterpret_cast(
  const T_Wrap<T_Rhs, T_Sbx>& rhs) noexcept
{
  static_assert(detail::rlbox_is_wrapper_v<T_Wrap<T_Rhs, T_Sbx>> &&
                  std::is_pointer_v<T_Lhs> && std::is_pointer_v<T_Rhs>,
                "sandbox_reinterpret_cast on incompatible types");

  tainted<T_Rhs, T_Sbx> taintedVal = rhs;
  auto raw = reinterpret_cast<T_Lhs>(taintedVal.UNSAFE_Unverified());
  auto ret = tainted<T_Lhs, T_Sbx>::internal_factory(raw);
  return ret;
}

template<typename T_Lhs,
         typename T_Rhs,
         typename T_Sbx,
         template<typename, typename>
         typename T_Wrap>
inline tainted<T_Lhs, T_Sbx> sandbox_const_cast(
  const T_Wrap<T_Rhs, T_Sbx>& rhs) noexcept
{
  static_assert(detail::rlbox_is_wrapper_v<T_Wrap<T_Rhs, T_Sbx>>,
                "sandbox_const_cast on incompatible types");

  tainted<T_Rhs, T_Sbx> taintedVal = rhs;
  auto raw = const_cast<T_Lhs>(taintedVal.UNSAFE_Unverified());
  auto ret = tainted<T_Lhs, T_Sbx>::internal_factory(raw);
  return ret;
}

namespace detail {
  // Checks that a given range is either entirely in a sandbox or entirely
  // outside
  // This is intentionally not const correct, so use with care
  template<typename T_Sbx, typename T_Ptr, typename T_Num>
  inline void* check_range_boundaries_get_start(RLBoxSandbox<T_Sbx>& sandbox,
                                                T_Ptr* ptr,
                                                T_Num size)
  {
    auto ptr_start_val = reinterpret_cast<uintptr_t>(ptr);
    auto ptr_end_val = ptr_start_val + size;

    auto ptr_start = reinterpret_cast<void*>(ptr_start_val);
    auto ptr_end = reinterpret_cast<void*>(ptr_end_val);

    detail::dynamic_check(sandbox.is_in_same_sandbox(ptr_start, ptr_end),
                          "range has overflowed sandbox bounds");

    return ptr_start;
  }
}

template<typename T_Sbx,
         typename T_Rhs,
         typename T_Val,
         typename T_Num,
         template<typename, typename>
         typename T_Wrap>
inline T_Wrap<T_Rhs*, T_Sbx> memset(RLBoxSandbox<T_Sbx>& sandbox,
                                    T_Wrap<T_Rhs*, T_Sbx> ptr,
                                    T_Val value,
                                    T_Num num)
{

  static_assert(detail::rlbox_is_tainted_or_vol_v<T_Wrap<T_Rhs, T_Sbx>>,
                "memset called on non wrapped type");

  static_assert(!std::is_const_v<T_Rhs>, "Destination is const");

  auto num_val = detail::unwrap_value(num);
  detail::dynamic_check(num_val <= sandbox.get_total_memory(),
                        "Called memset for memory larger than the sandbox");

  tainted<T_Rhs*, T_Sbx> ptr_tainted = ptr;
  void* dest_start = detail::check_range_boundaries_get_start(
    sandbox, ptr_tainted.UNSAFE_Unverified(), num_val);

  std::memset(dest_start, detail::unwrap_value(value), num_val);
  return ptr;
}

template<typename T_Sbx,
         typename T_Rhs,
         typename T_Lhs,
         typename T_Num,
         template<typename, typename>
         typename T_Wrap>
inline T_Wrap<T_Rhs*, T_Sbx> memcpy(RLBoxSandbox<T_Sbx>& sandbox,
                                    T_Wrap<T_Rhs*, T_Sbx> dest,
                                    T_Lhs src,
                                    T_Num num)
{

  static_assert(detail::rlbox_is_tainted_or_vol_v<T_Wrap<T_Rhs, T_Sbx>>,
                "memcpy called on non wrapped type");

  static_assert(!std::is_const_v<T_Rhs>, "Destination is const");

  auto num_val = detail::unwrap_value(num);
  detail::dynamic_check(num_val <= sandbox.get_total_memory(),
                        "Called memcpy for memory larger than the sandbox");

  tainted<T_Rhs*, T_Sbx> dest_tainted = dest;
  void* dest_start = detail::check_range_boundaries_get_start(
    sandbox, dest_tainted.UNSAFE_Unverified(), num_val);

  // src also needs to be checked, as we don't want to allow a src rand to start
  // inside the sandbox and end outside, and vice versa
  // src may or may not be a wrapper, so use unwrap_value
  void* src_start = detail::check_range_boundaries_get_start(
    sandbox, detail::unwrap_value(src), num_val);

  std::memcpy(dest_start, src_start, num_val);

  return dest;
}

}