/**
 * @file rlbox_tainted_fundamental_or_enum.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fundamental_or_enum class that adds
 * tainted/tainted_volatile support for fundamental (int, float, etc.) types and
 * enum types.
 */

#pragma once

#include <type_traits>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_type_conversion.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief Implementation of tainted/tainted volatile wrappers for fundamental
 * (int, float, etc.) types and enum types
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam TAppRep is the app representation of data
 * @tparam TSbxRep is the sandbox representation of data
 * @tparam TSbx is the type of sandbox
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_fundamental_or_enum
    : public tainted_any_base<TUseAppRep, TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 protected:
  static_assert(
      detail::is_fundamental_or_enum_v<TAppRep>,
      "tainted_fundamental_or_enum only supports fundamental_or_enum");

  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep =
      detail::tainted_rep_t<detail::rlbox_base_types_convertor<TAppRep, TSbx>>;

  /**
   * @brief The internal representation of data for this wrapper
   */
  using TIntRep = std::conditional_t<TUseAppRep, detail::tainted_rep_t<TAppRep>,
                                     detail::tainted_rep_t<TSbxRep>&>;

  TIntRep data;

  ////////////////////////////////

 public:
  /**
   * @brief Construct a new tainted fundamental or enum object
   * @tparam TAllowConstruction ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam RLBOX_REQUIRE enforces the public/private check
   */
  template <bool TAllowConstruction = TUseAppRep,
            RLBOX_REQUIRE(TAllowConstruction)>
  // NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
  inline tainted_fundamental_or_enum() : data(0) {
    // Can't use the default constructor as we need to check TAllowConstruction
  }

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TAllowConstruction ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam RLBOX_REQUIRE enforces the public/private check and also checks to
   * see if this meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <bool TAllowConstruction = TUseAppRep, bool TUseAppRepOther,
            typename TAppRepOther,
            RLBOX_REQUIRE(TAllowConstruction&& std::is_constructible_v<
                          detail::tainted_rep_t<TAppRep>, TAppRepOther>)>
  inline tainted_fundamental_or_enum(
      const tainted_fundamental_or_enum<TUseAppRepOther, TAppRepOther, TSbx>&
          aOther)
      : data([&] {
          if constexpr (TUseAppRep) {
            return aOther.raw_host_rep();
          } else {
            return aOther.raw_sandbox_rep();
          }
        }()) {}

  /**
   * @brief Construct a new tainted object from a raw primitive value
   * @tparam TAllowConstruction ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam TOther is the type of the rhs value
   * @tparam RLBOX_REQUIRE enforces the public/private check and also checks to
   * this is allowed for primitive types only and if this meets the
   * constructible criterion
   * @param aOther is the raw primitive
   */
  template <bool TAllowConstruction = TUseAppRep, typename TOther,
            RLBOX_REQUIRE(TAllowConstruction&& std::is_constructible_v<
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

  /**
   * @brief Construct a new tainted volatile object from a raw primitive value
   * @tparam TAllowConstruction ensures constructors are usable for tainted but
   * not tainted_volatile
   * @tparam TOther is the type of the rhs value
   * @tparam RLBOX_REQUIRE enforces the public/private check and also checks to
   * this is allowed for primitive types only and if this meets the
   * constructible criterion
   * @param aOther is the raw primitive
   */
  template <bool TAllowConstruction = !TUseAppRep,
            RLBOX_REQUIRE(TAllowConstruction&& std::is_constructible_v<
                          detail::tainted_rep_t<TAppRep>, TIntRep>)>
  inline tainted_fundamental_or_enum(const TIntRep& aOther) : data(aOther) {}

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
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified(
      [[maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
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
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed(
      [[maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_sandboxed();
  }

  ////////////////////////////////

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& is
   * the reference to this value
   */
  template <bool TUseAppRepOther, typename TAppRepOther,
            RLBOX_REQUIRE(std::is_assignable_v<detail::tainted_rep_t<TAppRep>&,
                                               TAppRepOther>)>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      const tainted_fundamental_or_enum<TUseAppRepOther, TAppRepOther, TSbx>&
          aOther) {
    if constexpr (TUseAppRep) {
      data = aOther.raw_host_rep();
    } else {
      data = aOther.raw_sandbox_rep();
    }
    return *this;
  }

  /**
   * @brief Operator= for tainted values from a raw primitive value
   * @tparam TOther is the type of the rhs value
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion
   * @param aOther is the raw primitive
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& is
   * the reference to this value
   */
  template <typename TOther,
            RLBOX_REQUIRE(
                std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TOther>)>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      const TOther& aOther) {
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

  // template <bool TUseAppRepOther>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      const tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& aOther) {
    if constexpr (TUseAppRep) {
      data = aOther.raw_host_rep();
    } else {
      data = aOther.raw_sandbox_rep();
    }
    return *this;
  }

 protected:
  template <typename T>
  using tainted = typename TSbx::template tainted<T>;

  /**
   * @brief Result type of operator&
   */
  using TOpAddrOf = std::conditional_t<
      TUseAppRep, tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>*,
      tainted<std::add_pointer_t<TAppRep>>>;

 public:
  /**
   * @brief Operator& which gets the address of tainted_volatile to get tainted
   * @return TOpAddrOf is the pointer to the sandbox memory that holds this data
   */
  inline TOpAddrOf operator&() {
    if constexpr (TUseAppRep) {
      return this;
    } else {
      // Deliberately use a C style cast as we we want to get rid of any CV
      // qualifers here. CV qualifiers are moved inside the wrapper type and
      // thus continue to be tracked.

      // NOLINTNEXTLINE(google-readability-casting)
      auto* data_ptr = (std::add_pointer_t<TAppRep>)&data;
      auto ret = TOpAddrOf::from_unchecked_raw_pointer(data_ptr);
      return ret;
    }
  }
};

}  // namespace rlbox
