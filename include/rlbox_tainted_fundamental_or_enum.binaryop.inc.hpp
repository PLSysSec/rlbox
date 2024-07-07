/**
 * @file rlbox_tainted_fundamental_or_enum.compareop.inc.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the comparison operators for the
 * tainted_fundamental_or_enum_base class. This file can be included multiple
 * times with after defining the macro RLBOX_BINARY_OP to a binary operator
 * like +, -, * etc.
 */

#ifndef RLBOX_BINARY_OP
// if RLBOX_BINARY_OP is not defined, we want this file to parse sensibly for
// when read on its own
#  define RLBOX_PARSE_FOR_STANDALONE
#endif

#ifdef RLBOX_PARSE_FOR_STANDALONE
#  define RLBOX_BINARY_OP +
#  include "rlbox_helpers.hpp"
#  include "rlbox_types.hpp"
// IWYU doesn't seem to recognize the call to RLBOX_REQUIRE so force IWYU to
// keep the next include
#  include "rlbox_wrapper_traits.hpp"  // IWYU pragma: keep

namespace rlbox {
template <bool TUseAppRep, typename TData, typename TSbx>
class dummy {
  using this_t = dummy<TUseAppRep, TData, TSbx>;
  TData data{0};
#endif

  /**
   * @brief Operator on the current tainted value and another tainted/primitive
   * value
   * @tparam TArg is the rhs tainted/primitive type
   * @param aLhs is the lhs argument which is a this_t value
   * @param aRhs is the rhs argument which is a tainted/primitive value
   * @return a tainted result
   */
  template <typename TArg>
  friend inline constexpr auto
  operator RLBOX_BINARY_OP(const this_t& aLhs, const TArg& aRhs) noexcept(
      noexcept(aLhs.raw_host_rep() RLBOX_BINARY_OP aLhs.raw_host_rep())) {
    TAppRep result = 0;
    if constexpr (detail::is_tainted_any_wrapper_v<TArg>) {
      result = aLhs.raw_host_rep() RLBOX_BINARY_OP aRhs.raw_host_rep();
    } else {
      result = aLhs.raw_host_rep() RLBOX_BINARY_OP aRhs;
    }

    return tainted<TData, TSbx>(result);
  }

  /**
   * @brief Operator on a primitive value and a tainted value
   * @tparam TArg is the lhs tainted type
   * @tparam RLBOX_REQUIRE checks if the lhs is not a tainted_* wrapper
   * @param aLhs is the lhs argument which is a primitive value
   * @param aRhs is the rhs argument which is a tainted value
   * @return a tainted result
   */
  template <typename TArg,
            RLBOX_REQUIRE(!detail::is_tainted_any_wrapper_v<TArg>)>
  friend inline constexpr auto
  operator RLBOX_BINARY_OP(const TArg& aLhs, const this_t& aRhs) noexcept(
      noexcept(aRhs.raw_host_rep() RLBOX_BINARY_OP aRhs.raw_host_rep())) {
    TAppRep result = aLhs RLBOX_BINARY_OP aRhs.raw_host_rep();
    return tainted<TData, TSbx>(result);
  }

#define RLBOX_CONCAT_HELPER2(a) a## =
#define RLBOX_CONCAT_HELPER(a) RLBOX_CONCAT_HELPER2(a)
#define RLBOX_BINARY_ASSIGN_OP RLBOX_CONCAT_HELPER(RLBOX_BINARY_OP)

  /**
   * @brief Operator to modify and assign the current tainted value using a
   * tainted/primitive value
   * @tparam TArg is the rhs tainted/primitive type
   * @param aRhs is the rhs argument which is a tainted/primitive value
   * @return a tainted result
   */
  template <typename TArg>
  inline constexpr this_t& operator RLBOX_BINARY_ASSIGN_OP(
      const TArg & aRhs) noexcept(noexcept(data RLBOX_BINARY_ASSIGN_OP data)) {
    if constexpr (detail::is_tainted_any_wrapper_v<TArg>) {
      data RLBOX_BINARY_ASSIGN_OP aRhs.raw_host_rep();
    } else {
      data RLBOX_BINARY_ASSIGN_OP aRhs;
    }

    return *this;
  }

#ifdef RLBOX_PARSE_FOR_STANDALONE
};
}
#endif

#undef RLBOX_CONCAT_HELPER2
#undef RLBOX_CONCAT_HELPER
#undef RLBOX_PARSE_FOR_STANDALONE
#undef RLBOX_BINARY_ASSIGN_OP
#undef RLBOX_BINARY_OP
