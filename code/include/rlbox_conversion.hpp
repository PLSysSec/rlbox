#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <array>
#include <cstring>
#include <limits>
#include <type_traits>

#include "rlbox_helpers.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox::detail {

template<typename T_To, typename T_From>
inline constexpr void adjust_type_size_fundamental(T_To& to, const T_From& from)
{
  using namespace std;

  if_constexpr_named(cond1, !is_fundamental_or_enum_v<T_To>)
  {
    rlbox_detail_static_fail_because(
      cond1, "Conversion target should be fundamental or enum type");
  }
  else if_constexpr_named(cond2, !is_fundamental_or_enum_v<T_From>)
  {
    rlbox_detail_static_fail_because(
      cond2, "Conversion source should be fundamental or enum type");
  }
  else if_constexpr_named(cond3, is_enum_v<T_To> || is_enum_v<T_From>)
  {
    static_assert(std::is_same_v<T_To, T_From>);
    to = from;
  }
  else if_constexpr_named(
    cond4, is_floating_point_v<T_To> || is_floating_point_v<T_From>)
  {
    static_assert(is_floating_point_v<T_To> && is_floating_point_v<T_From>);
    // language coerces different float types
    to = from;
  }
  else if_constexpr_named(cond5, is_integral_v<T_To> || is_integral_v<T_From>)
  {
    static_assert(is_integral_v<T_To> && is_integral_v<T_From>);

    const char* err_msg =
      "Over/Underflow when converting between integer types";

    if constexpr (is_signed_v<T_To> == is_signed_v<T_From> &&
                  sizeof(T_To) >= sizeof(T_From)) {
      // Eg: int64_t from int32_t, uint64_t from uint32_t
    } else if constexpr (is_unsigned_v<T_To> && is_unsigned_v<T_From>) {
      // Eg: uint32_t from uint64_t
      dynamic_check(from <= numeric_limits<T_To>::max(), err_msg);
    } else if constexpr (is_signed_v<T_To> && is_signed_v<T_From>) {
      // Eg: int32_t from int64_t
      dynamic_check(from >= numeric_limits<T_To>::min(), err_msg);
      dynamic_check(from <= numeric_limits<T_To>::max(), err_msg);
    } else if constexpr (is_unsigned_v<T_To> && is_signed_v<T_From>) {
      if constexpr (sizeof(T_To) < sizeof(T_From)) {
        // Eg: uint32_t from int64_t
        dynamic_check(from >= 0, err_msg);
        auto to_max = numeric_limits<T_To>::max();
        dynamic_check(from <= static_cast<T_From>(to_max), err_msg);
      } else {
        // Eg: uint32_t from int32_t, uint64_t from int32_t
        dynamic_check(from >= 0, err_msg);
      }
    } else if constexpr (is_signed_v<T_To> && is_unsigned_v<T_From>) {
      if constexpr (sizeof(T_To) <= sizeof(T_From)) {
        // Eg: int32_t from uint32_t, int32_t from uint64_t
        auto to_max = numeric_limits<T_To>::max();
        dynamic_check(from <= static_cast<T_From>(to_max), err_msg);
      } else {
        // Eg: int64_t from uint32_t
      }
    }
    to = static_cast<T_To>(from);
  }
  else
  {
    constexpr auto unknownCase = !(cond1 || cond2 || cond3 || cond4 || cond5);
    rlbox_detail_static_fail_because(unknownCase,
                                     "Unexpected case for adjust_type_size");
  }
}

template<typename T_To, typename T_From>
inline constexpr void adjust_type_size_fundamental_or_array(T_To& to,
                                                            const T_From& from)
{
  using namespace std;

  using T_To_C = std_array_to_c_arr_t<T_To>;
  using T_From_C = std_array_to_c_arr_t<T_From>;
  using T_To_El = remove_all_extents_t<T_To_C>;
  using T_From_El = remove_all_extents_t<T_From_C>;

  if_constexpr_named(cond1, is_array_v<T_To_C> != is_array_v<T_From_C>)
  {
    rlbox_detail_static_fail_because(
      cond1, "Conversion should not go between array and non array types");
  }
  else if constexpr (!is_array_v<T_To_C>)
  {
    return adjust_type_size_fundamental(to, from);
  }
  else if_constexpr_named(cond2, !all_extents_same<T_To_C, T_From_C>)
  {
    rlbox_detail_static_fail_because(
      cond2, "Conversion between arrays should have same dimensions");
  }
  else if_constexpr_named(cond3,
                          is_pointer_v<T_To_El> || is_pointer_v<T_From_El>)
  {
    rlbox_detail_static_fail_because(
      cond3, "adjust_type_size does not allow arrays of pointers");
  }
  else
  {
    // Explicitly using size to check for element type as we may be going across
    // different types of the same width such as void* and uintptr_t
    if constexpr (sizeof(T_To_El) == sizeof(T_From_El) &&
                  is_signed_v<T_To_El> == is_signed_v<T_From_El>) {
      // Sanity check - this should definitely be true
      static_assert(sizeof(T_From_C) == sizeof(T_To_C));
      memcpy(&to, &from, sizeof(T_To_C));
    } else {
      for (size_t i = 0; i < std::extent_v<T_To_C>; i++) {
        adjust_type_size_fundamental_or_array(to[i], from[i]);
      }
    }
  }
}

enum class adjust_type_direction
{
  TO_SANDBOX,
  TO_APPLICATION,
  NO_CHANGE
};

template<typename T_Sbx,
         adjust_type_direction Direction,
         typename T_To,
         typename T_From>
inline constexpr void adjust_type_size(T_To& to,
                                       const T_From& from,
                                       const void* example_unsandboxed_ptr)
{
  using namespace std;

  using T_To_C = std_array_to_c_arr_t<T_To>;
  using T_From_C = std_array_to_c_arr_t<T_From>;
  using T_To_El = remove_all_extents_t<T_To_C>;
  using T_From_El = remove_all_extents_t<T_From_C>;

  if constexpr (is_pointer_v<T_To_C> || is_pointer_v<T_From_C>) {

    if constexpr (Direction == adjust_type_direction::NO_CHANGE) {

      static_assert(is_pointer_v<T_To_C> && is_pointer_v<T_From_C> &&
                    sizeof(T_To_C) == sizeof(T_From_C));
      to = from;

    } else if constexpr (Direction == adjust_type_direction::TO_SANDBOX) {

      static_assert(is_pointer_v<T_From_C>);
      to = RLBoxSandbox<T_Sbx>::template get_sandboxed_pointer_no_ctx<
        remove_pointer_t<T_From_C>>(from);

    } else if constexpr (Direction == adjust_type_direction::TO_APPLICATION) {

      static_assert(is_pointer_v<T_To_C>);
      to = RLBoxSandbox<T_Sbx>::template get_unsandboxed_pointer_no_ctx<
        remove_pointer_t<T_To_C>>(from, example_unsandboxed_ptr);
    }

  } else if constexpr (is_pointer_v<T_To_El> || is_pointer_v<T_From_El>) {

    static_assert(is_pointer_v<T_To_El> && is_pointer_v<T_From_El>);

    if constexpr (Direction == adjust_type_direction::NO_CHANGE) {
      // Sanity check - this should definitely be true
      static_assert(sizeof(T_To_El) == sizeof(T_From_El) &&
                    sizeof(T_From_C) == sizeof(T_To_C));
      memcpy(&to, &from, sizeof(T_To_C));
    } else {
      for (size_t i = 0; i < std::extent_v<T_To_C>; i++) {
        adjust_type_size(to[i], from[i]);
      }
    }

  } else {
    adjust_type_size_fundamental_or_array(to, from);
  }
}

template<typename T_Sbx,
         adjust_type_direction Direction,
         typename T_To,
         typename T_From>
inline constexpr void adjust_type_size(T_To& to, const T_From& from)
{
  static_assert(
    Direction == adjust_type_direction::NO_CHANGE ||
      Direction == adjust_type_direction::TO_SANDBOX,
    "Example pointer cannot be ommitted for direction TO_APPLICATION");
  adjust_type_size<T_Sbx, Direction>(
    to, from, nullptr /* example_unsandboxed_ptr */);
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