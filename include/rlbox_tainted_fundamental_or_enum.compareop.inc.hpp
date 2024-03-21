/**
 * @file rlbox_tainted_fundamental_or_enum.compareop.inc.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the comparison operators for the
 * tainted_fundamental_or_enum_base class. This file can be included multiple
 * times with after defining the macro RLBOX_COMPARE_OP to a comparison operator
 * like ==, !=, <= etc.
 */

#ifndef RLBOX_COMPARE_OP
// if RLBOX_COMPARE_OP is not defined, we want this file to parse sensibly for
// intellisense define a value for intellisense
#  define RLBOX_PARSE_FOR_INTELLISENSE
#endif

#ifdef RLBOX_PARSE_FOR_INTELLISENSE
#  define RLBOX_COMPARE_OP ==
#  include "rlbox_helpers.hpp"
#  include "rlbox_types.hpp"
// IWYU doesn't seem to recognize the call to RLBOX_REQUIRE so force IWYU to
// keep the next include
#  include "rlbox_wrapper_traits.hpp"  // IWYU pragma: keep

namespace rlbox {
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class dummy {
  using this_t = dummy<TUseAppRep, TAppRep, TSbx>;
#endif

  /**
   * @brief Operator compares this_t value against another tainted_* value
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the rhs AppRep
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam TExtraOther... is the extra args of the rhs
   * @tparam RLBOX_REQUIRE checks if the rhs is a tainted_* wrapper
   * @param aLhs is the lhs argument which is a this_t value
   * @param aRhs is the rhs argument which is a tainted_* value
   * @return a @ref rlbox::tainted_boolean_hint if either `aLhs` or `aRhs` is
   * @ref rlbox::tainted_volatile wrapper, bool otherwise
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<TWrap<
                        TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>>)>
  friend inline constexpr auto operator RLBOX_COMPARE_OP(
      const this_t& aLhs,
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>&
          aRhs) noexcept(noexcept(aLhs.raw_host_rep()
                                      RLBOX_COMPARE_OP aRhs.raw_host_rep() &&
                                  aLhs.raw_sandbox_rep() RLBOX_COMPARE_OP aRhs
                                      .raw_sandbox_rep())) {
    bool ret = false;

    // If both the lhs and rhs are in the sandbox representation, directly
    // compare those. Else convert both sides to app representation (this is a
    // noop for each side that is already in the app representation)
    if constexpr (!TUseAppRep && !TUseAppRepOther) {
      ret = aLhs.raw_sandbox_rep() RLBOX_COMPARE_OP aRhs.raw_sandbox_rep();
    } else {
      ret = aLhs.raw_host_rep() RLBOX_COMPARE_OP aRhs.raw_host_rep();
    }

    if constexpr (TUseAppRep && TUseAppRepOther) {
      return ret;
    } else {
      return tainted_boolean_hint<TSbx>(ret);
    }
  }

  /**
   * @brief Operator compares this_t value against a primitive value
   * @tparam TArg is the rhs primitive type
   * @tparam RLBOX_REQUIRE checks if the rhs is not a tainted_* wrapper
   * @param aLhs is the lhs argument which is a this_t value
   * @param aRhs is the rhs argument which is a primitive value
   * @return a @ref rlbox::tainted_boolean_hint if `aLhs` is @ref
   * rlbox::tainted_volatile wrapper, bool otherwise
   */
  template <typename TArg,
            RLBOX_REQUIRE(!detail::is_tainted_any_wrapper_v<TArg>)>
  friend inline constexpr auto operator RLBOX_COMPARE_OP(
      const this_t& aLhs,
      const TArg& aRhs) noexcept(noexcept(aLhs.raw_host_rep()
                                              RLBOX_COMPARE_OP aRhs)) {
    const bool ret = aLhs.raw_host_rep() RLBOX_COMPARE_OP aRhs;
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint<TSbx>(ret);
    }
  }

  /**
   * @brief Operator compares s primitive value against this_t value
   * @tparam TArg is the lhs primitive type
   * @tparam RLBOX_REQUIRE checks if the lhs is not a tainted_* wrapper
   * @param aLhs is the lhs argument which is a primitive value
   * @param aRhs is the rhs argument which is a this_t value
   * @return a @ref rlbox::tainted_boolean_hint if `aRhs` is @ref
   * rlbox::tainted_volatile wrapper, bool otherwise
   */
  template <typename TArg,
            RLBOX_REQUIRE(!detail::is_tainted_any_wrapper_v<TArg>)>
  friend inline constexpr auto
  operator RLBOX_COMPARE_OP(const TArg& aLhs, const this_t& aRhs) noexcept(
      noexcept(aLhs RLBOX_COMPARE_OP aRhs.raw_host_rep())) {
    const bool ret = aLhs RLBOX_COMPARE_OP aRhs.raw_host_rep();
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint<TSbx>(ret);
    }
  }

#ifdef RLBOX_PARSE_FOR_INTELLISENSE
};
}
#endif

#undef RLBOX_PARSE_FOR_INTELLISENSE
#undef RLBOX_COMPARE_OP