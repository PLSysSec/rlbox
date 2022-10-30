/**
 * @file rlbox_tainted_common.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief These classes contain common functions used by different
 * tainted and tainted_volatile implementations.
 */

#pragma once

#include <type_traits>

#include "rlbox_helpers.hpp"
#include "rlbox_type_conversion.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief Implementation of tainted/tainted volatile wrappers for fundamental
 * (int, float, etc.) types and enum types
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation or the sandbox representation
 * @tparam TAppRep is the app representation of data
 * @tparam TSbxRep is the sandbox representation of data
 * @tparam TSbx is the type of sandbox
 */
template <bool TUseAppRep, typename TAppRep, typename TSbxRep, typename TSbx>
class tainted_fundamental_or_enum {
 protected:
  static_assert(
      detail::is_fundamental_or_enum_v<TAppRep>,
      "tainted_fundamental_or_enum only supports fundamental_or_enum");

  /**
   * @brief The internal representation of data for this wrapper
   */
  using TIntRep = std::conditional_t<TUseAppRep, TAppRep, TSbxRep>;

  detail::tainted_rep_t<TIntRep> data{0};

  ////////////////////////////////

 public:
  /**
   * @brief Construct a new tainted fundamental or enum object
   * @tparam TAllowPublicConst ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam RLBOX_REQUIRE enforces the public/private check
   */
  template <bool TAllowPublicConst = TUseAppRep,
            RLBOX_REQUIRE(TAllowPublicConst)>
  // NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
  inline tainted_fundamental_or_enum() {
    // Can't use the default constructor as we need to check TAllowPublicConst
  }

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TAllowPublicConst ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam TSbxRepOther is the TSbxRep of the rhs value
   * @tparam RLBOX_REQUIRE enforces the public/private check and also checks to
   * see if this meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <bool TAllowPublicConst = TUseAppRep, bool TUseAppRepOther,
            typename TAppRepOther, typename TSbxRepOther,
            RLBOX_REQUIRE(TAllowPublicConst&& std::is_constructible_v<
                          detail::tainted_rep_t<TAppRep>, TAppRepOther>)>
  inline tainted_fundamental_or_enum(
      const tainted_fundamental_or_enum<TUseAppRepOther, TAppRepOther,
                                        TSbxRepOther, TSbx>& aOther)
      : data([&] {
          if constexpr (TUseAppRep) {
            return aOther.raw_host_rep();
          } else {
            return aOther.raw_sandbox_rep();
          }
        }()) {}

  /**
   * @brief Construct a new tainted object from a raw primitive value
   * @tparam TAllowPublicConst ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE enforces the public/private check and also checks to
   * this is allowed for primitive types only and if this meets the
   * constructible criterion
   * @param aOther is the raw primitive
   */
  template <bool TAllowPublicConst = TUseAppRep, typename TOther,
            RLBOX_REQUIRE(TAllowPublicConst&& std::is_constructible_v<
                          detail::tainted_rep_t<TAppRep>, TOther>)>
  inline tainted_fundamental_or_enum(const TOther& aOther)
      : data([&] {
          if constexpr (TUseAppRep) {
            return aOther;
          } else {
            return detail::convert_type_fundamental<
                detail::tainted_rep_t<TSbxRep>>(aOther);
          }
        }()) {}

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified()
      const {
    if constexpr (TUseAppRep) {
      return data;
    } else {
      auto converted =
          detail::convert_type_fundamental<detail::tainted_rep_t<TAppRep>>(
              data);
      return converted;
    }
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_unverified();
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed() const {
    if constexpr (TUseAppRep) {
      auto converted =
          detail::convert_type_fundamental<detail::tainted_rep_t<TSbxRep>>(
              data);
      return converted;
    } else {
      return data;
    }
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to
   the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed
   ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_sandboxed();
  }

  ////////////////////////////////

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam TSbxRepOther is the TSbxRep of the rhs value
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbxRep, TSbx>& is
   * the reference to this value
   */
  template <bool TUseAppRepOther, typename TAppRepOther, typename TSbxRepOther,
            RLBOX_REQUIRE(std::is_assignable_v<detail::tainted_rep_t<TAppRep>&,
                                               TAppRepOther>)>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbxRep, TSbx>&
  operator=(const tainted_fundamental_or_enum<TUseAppRepOther, TAppRepOther,
                                              TSbxRepOther, TSbx>& aOther) {
    if constexpr (TUseAppRep) {
      data = aOther.raw_host_rep();
    } else {
      data = aOther.raw_sandbox_rep();
    }
    return *this;
  }

  /**
   * @brief
   * @tparam TOther Operator= for tainted values from a raw primitive value
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion
   * @param aOther is the raw primitive
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbxRep, TSbx>& is
   * the reference to this value
   */
  template <typename TOther,
            RLBOX_REQUIRE(
                std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TOther>)>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbxRep, TSbx>&
  operator=(const TOther& aOther) {
    if constexpr (TUseAppRep) {
      data = aOther;
    } else {
      auto converted =
          detail::convert_type_fundamental<detail::tainted_rep_t<TSbxRep>>(
              aOther);
      data = converted;
    }
    return *this;
  }
};

}  // namespace rlbox
