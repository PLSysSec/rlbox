/**
 * @file rlbox_tainted_fundamental_or_enum.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fundamental_or_enum class that adds
 * tainted/tainted_volatile support for fundamental (int, float, etc.) types and
 * enum types.
 */

#pragma once

#include <type_traits>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_data_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_tainted_hint.hpp"
#include "rlbox_type_traits.hpp"
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
      detail::is_fundamental_or_enum_v<detail::tainted_rep_t<TAppRep>>,
      "tainted_fundamental_or_enum only supports fundamental_or_enum");

  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep =
      detail::tainted_rep_t<detail::rlbox_base_types_convertor<TAppRep, TSbx>>;

  /**
   * @brief The internal representation of data for this wrapper
   */
  using TRep = std::conditional_t<TUseAppRep, detail::tainted_rep_t<TAppRep>,
                                  detail::tainted_rep_t<TSbxRep>>;

  /**
   * @brief The tainted_boolean_hint type specialized for this sandbox.
   */
  using tainted_boolean_hint_t = rlbox::tainted_boolean_hint<true, bool, TSbx>;

  /**
   * @brief The current class's type
   */
  using this_t = tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>;

  TRep data{0};

  ////////////////////////////////

 public:
  /**
   * @brief Construct a new tainted_fundamental_or_enum object
   */
  inline tainted_fundamental_or_enum() = default;
  /**
   * @brief Copy constructor: Construct a new tainted_fundamental_or_enum object
   */
  inline tainted_fundamental_or_enum(
      const tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>&) = default;
  /**
   * @brief Move constructor: Construct a new tainted_fundamental_or_enum object
   */
  inline tainted_fundamental_or_enum(
      tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>&&) noexcept =
      default;

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam RLBOX_REQUIRE checks if (1) this won't be handled the original
   * class's copy/move constructor and (2) this meets the constructible
   * criterion
   * @param aOther is the rhs being assigned
   */
  template <
      template <bool, typename, typename> typename TWrap, bool TUseAppRepOther,
      typename TAppRepOther,
      RLBOX_REQUIRE(!std::is_same_v<
                        tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>,
                        TWrap<TUseAppRepOther, TAppRepOther, TSbx>> &&
                    std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
                                            TAppRepOther>)>
  inline tainted_fundamental_or_enum(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx>& aOther)
      : data([&] {
          if constexpr (TUseAppRep) {
            return aOther.raw_host_rep();
          } else {
            return aOther.raw_sandbox_rep();
          }
        }()) {}

  /**
   * @brief Construct a new tainted object from a raw (unwrapped) primitive
   * value
   * @tparam TOther is the type of the rhs value
   * @tparam RLBOX_REQUIRE checks if this meets the constructible criterio
   * @param aOther is the raw primitive
   */
  template <typename TOther,
            RLBOX_REQUIRE(std::is_constructible_v<
                          detail::tainted_rep_t<TAppRep>, TOther>)>
  inline tainted_fundamental_or_enum(const TOther& aOther)
      : data([&] {
          if constexpr (TUseAppRep) {
            return aOther;
          } else {
            detail::tainted_rep_t<detail::remove_cvref_t<TSbxRep>> ret;
            detail::convert_type_fundamental(&ret, aOther);
            return ret;
          }
        }()) {}

  /**
   * @brief Destroy the tainted_fundamental_or_enum object
   */
  inline ~tainted_fundamental_or_enum() = default;

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
      detail::tainted_rep_t<detail::remove_cvref_t<TAppRep>> converted;
      detail::convert_type_fundamental(&converted, data);
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
      detail::tainted_rep_t<detail::remove_cvref_t<TSbxRep>> converted;
      detail::convert_type_fundamental(&converted, data);
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
   * @brief Copy assignment operator
   * @param aOther is the rhs argument
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& returns
   * this object
   */
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      const tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>&
          aOther) noexcept = default;

  /**
   * @brief Move assignment operator
   * @param aOther is the rhs argument
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& returns
   * this object
   */
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>&&
          aOther) noexcept = default;

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the TAppRep of the rhs value
   * @tparam RLBOX_REQUIRE checks if (1) this won't be handled the original
   * class's copy/move assignment and (2) this meets the assignable
   * criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& is
   * the reference to this value
   */
  template <
      template <bool, typename, typename> typename TWrap, bool TUseAppRepOther,
      typename TAppRepOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx>> &&
          !detail::is_same_wrapper_type_v<TWrap, TUseAppRepOther, TAppRepOther,
                                          TSbx, this_t> &&
          std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TAppRepOther>)>
  inline tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>& operator=(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx>& aOther) noexcept {
    if constexpr (TUseAppRep) {
      data = aOther.raw_host_rep();
    } else {
      data = aOther.raw_sandbox_rep();
    }
    return *this;
  }

  /**
   * @brief Operator= for tainted values from a raw (unwrapped) primitive value
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
      detail::convert_type_fundamental(&data, aOther);
    }
    return *this;
  }

  ////////////////////////////////

 protected:
  using TCompareRet =
      std::conditional_t<TUseAppRep, bool, tainted_boolean_hint_t>;

 public:
  /**
   * @brief Operator== compares a primtive value against this tainted value
   * @tparam TArg is the type of the rhs value
   * @tparam RLBOX_REQUIRE checks to see if the argument is not a tainted value
   * @param aThis is the lhs argument whose type is this class
   * @param aArg is the rhs argument
   * @return `bool` if the value wrapped is a `tainted` value, and a @ref
   * rlbox::tainted_boolean_hint if the value wrapped is a `tainted_volatile`
   * value
   */
  template <typename TArg, RLBOX_REQUIRE(!detail::is_tainted_any_wrapper_v<
                                         std::remove_reference_t<TArg>>)>
  friend inline TCompareRet operator==(const this_t& aThis, TArg&& aArg) {
    const bool ret = aThis.UNSAFE_unverified() == aArg;
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint_t(ret);
    }
  }

  /**
   * @brief Operator!= compares a primtive value against this tainted value
   * @tparam TArg is the type of the rhs value
   * @tparam RLBOX_REQUIRE checks to see if the argument is not a tainted value
   * @param aThis is the lhs argument whose type is this class
   * @param aArg is the rhs argument
   * @return `bool` if the value wrapped is a `tainted` value, and a @ref
   * rlbox::tainted_boolean_hint if the value wrapped is a `tainted_volatile`
   * value
   */
  template <typename TArg, RLBOX_REQUIRE(!detail::is_tainted_any_wrapper_v<
                                         std::remove_reference_t<TArg>>)>
  friend inline TCompareRet operator!=(const this_t& aThis, TArg&& aArg) {
    const bool ret = aThis.UNSAFE_unverified() != aArg;
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint_t(ret);
    }
  }

  /**
   * @brief Operator== compares a tainted value against this tainted value
   * @tparam TArg is the type of the rhs tainted value
   * @tparam RLBOX_REQUIRE checks to see if the argument is not a tainted value
   * @param aThis is the lhs argument whose type is this class
   * @param aArg is the rhs argument which is a tainted value
   * @return `bool` if this class is a `tainted` value, and a @ref
   * rlbox::tainted_boolean_hint if the value wrapped is a `tainted_volatile`
   * value
   */
  template <typename TArg, RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<
                                         std::remove_reference_t<TArg>>)>
  friend inline TCompareRet operator==(const this_t& aThis, TArg&& aArg) {
    const bool ret = aThis.UNSAFE_unverified() == aArg.UNSAFE_unverified();
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint_t(ret);
    }
  }

  /**
   * @brief Operator!= compares a tainted value against this tainted value
   * @tparam TArg is the type of the rhs tainted value
   * @tparam RLBOX_REQUIRE checks to see if the argument is not a tainted value
   * @param aThis is the lhs argument whose type is this class
   * @param aArg is the rhs argument which is a tainted value
   * @return `bool` if this class is a `tainted` value, and a @ref
   * rlbox::tainted_boolean_hint if the value wrapped is a `tainted_volatile`
   * value
   */
  template <typename TArg, RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<
                                         std::remove_reference_t<TArg>>)>
  friend inline TCompareRet operator!=(const this_t& aThis, TArg&& aArg) {
    const bool ret = aThis.UNSAFE_unverified() != aArg.UNSAFE_unverified();
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint_t(ret);
    }
  }

  /**
   * @brief This conversion operator allows conversion of this tainted value to
   * a `bool` if this class is a `tainted` value, and a @ref
   * rlbox::tainted_boolean_hint if the value wrapped is a `tainted_volatile`
   * value
   * @return The converted value
   */
  explicit inline operator TCompareRet() const {
    const bool ret = data != 0;
    if constexpr (TUseAppRep) {
      return ret;
    } else {
      return tainted_boolean_hint_t(ret);
    }
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

  ////////////////////////////////

  /**
   * @brief prefix increment operator for tainted fundamental
   * @return the incremented value
   */
  this_t& operator++() {
    ++data;
    return *this;
  }

  /**
   * @brief postfix increment operator for tainted fundamental
   * @return the value prior to increment
   */
  this_t operator++(int) {
    this_t old = *this;
    operator++();
    return old;
  }

  /**
   * @brief prefix decrement operator for tainted fundamental
   * @return the decremented value
   */
  this_t& operator--() {
    --data;
    return *this;
  }

  /**
   * @brief postfix decrement operator for tainted fundamental
   * @return the value prior to decrement
   */
  this_t operator--(int) {
    this_t old = *this;
    operator--();
    return old;
  }
};

}  // namespace rlbox
