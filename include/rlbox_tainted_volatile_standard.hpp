/**
 * @file rlbox_tainted_volatile_standard.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_volatile_standard type.
 */

#pragma once

#include <stddef.h>
#include <type_traits>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_tainted_fundamental_or_enum.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {
/**
 * @brief Implementation of tainted_volatile data wrapper for pointer data. This
 * wrapper indicates that this data is located in memory that can be modified by
 * the sandbox.
 *
 * @details tainted_volatile_standard_pointer data in addition to being
 * untrusted (since it is memory written to be sandboxed code), should be
 * carefully checked for double-read or time-of-check-time-of-use style bugs,
 * i.e., if this data is read twice, the data could have been changed
 * asynchronously by a concurrent sandbox thread. To safely handle
 * tainted_volatile_standard_pointer data, the host application should make a
 * copy of tainted_volatile_standard_pointer data into the application memory
 * (tainted data) or call copy_and_verify to make a sanitized copy of this data.
 *
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename T, typename TSbx>
class tainted_volatile_standard_pointer
    : public tainted_any_base<false, T, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

  static_assert(!TUseAppRep, "Expected TUseAppRep to be false");
  static_assert(std::is_pointer_v<T>, "Expected T to be a pointer");

 private:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<T, TSbx>;

  // NOLINTNEXTLINE(hicpp-use-nullptr, modernize-use-nullptr)
  detail::tainted_rep_t<TSbxRep> data{0};

  ////////////////////////////////
  /**
   * @brief Construct a tainted volatile pointer set to null
   */
  inline tainted_volatile_standard_pointer() = default;

  /**
   * @brief Construct a new tainted object from another tainted wrapper
   * @tparam TWrap is the other wrapper type
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this is (1) a tainted wrapper,
   * (2) meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <template <bool, typename, typename> typename TWrap,
            bool TUseAppRepOther, typename TOther,
            RLBOX_REQUIRE(
                detail::is_tainted_any_wrapper_v<
                    TWrap<TUseAppRepOther, TOther, TSbx>, TSbx>&&
                    std::is_constructible_v<detail::tainted_rep_t<T>, TOther>)>
  inline tainted_volatile_standard_pointer(
      const TWrap<TUseAppRepOther, TOther, TSbx>& aOther)
      : data(aOther.raw_sandbox_rep()) {}

  /**
   * @brief Construct a tainted value with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_volatile_standard_pointer([
      [maybe_unused]] const std::nullptr_t& aNull)
      : data(0) {}

 public:
  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<T> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<T> UNSAFE_unverified([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return aSandbox.get_unsandboxed_pointer(data);
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data in the sandboxed
   * ABI.
   * @return detail::tainted_rep_t<TSbxRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed() const {
    return data;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_sandboxed();
  }

  ////////////////////////////////

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the other wrapper type
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this is (1) a tainted wrapper,
   * (2) meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return tainted_volatile_standard_pointer<T, TSbx>& is the reference to
   * this value
   */
  template <template <bool, typename, typename> typename TWrap,
            bool TUseAppRepOther, typename TOther,
            RLBOX_REQUIRE(
                detail::is_tainted_any_wrapper_v<
                    TWrap<TUseAppRepOther, TOther, TSbx>, TSbx>&&
                    std::is_assignable_v<detail::tainted_rep_t<T>&, TOther>)>
  inline tainted_volatile_standard_pointer<TUseAppRep, T, TSbx>& operator=(
      const TWrap<TUseAppRepOther, TOther, TSbx>& aOther) {
    data = aOther->raw_sandbox_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_volatile_standard_pointer<TUseAppRep, T, TSbx>& operator=([
      [maybe_unused]] const std::nullptr_t& aNull) {
    data = 0;
    return *this;
  }

  ////////////////////////////////

 private:
  template <typename TSub>
  using tainted_volatile = typename TSbx::template tainted_volatile<TSub>;

  using TOpDeref = tainted_volatile<std::remove_pointer_t<T>>;

 public:
  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE ensures this is allowed for pointer types only.
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref& operator*() {
    /// \todo eliminate cast and replace with tainted_volatile constructor
    /// taking a reference

    // Deliberately use a C style cast as we we want to get rid of any CV
    // qualifers here. CV qualifiers are moved inside the wrapper type and thus
    // continue to be tracked.

    // NOLINTNEXTLINE(google-readability-casting)
    auto data_tainted_volatile = (TOpDeref*)data;
    return *data_tainted_volatile;
  }

 private:
  template <typename TSub>
  using tainted = typename TSbx::template tainted<TSub>;

  using TOpAddrOf = tainted<std::add_pointer_t<T>>;

 public:
  /**
   * @brief Operator& which gets the address of tainted_volatile to get tainted
   * @return TOpAddrOf is the pointer to the sandbox memory that holds this data
   */
  inline TOpAddrOf operator&() {
    // Deliberately use a C style cast as we we want to get rid of any CV
    // qualifers here. CV qualifiers are moved inside the wrapper type and thus
    // continue to be tracked.

    // NOLINTNEXTLINE(google-readability-casting)
    auto* data_ptr = (std::add_pointer_t<T>)&data;
    auto ret = TOpAddrOf::from_unchecked_raw_pointer(data_ptr);
    return ret;
  }
};

/**
 * @brief Implementation of tainted_volatile data wrapper for pointer data. This
 * wrapper indicates that this data is located in memory that can be modified by
 * the sandbox.
 *
 * @details This is implemented using @ref rlbox::tainted_fundamental_or_enum
 * for fundamental or enum types, @ref rlbox::tainted_volatile_standard_pointer
 * for pointer types.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
using tainted_volatile_standard = std::conditional_t<
    detail::is_fundamental_or_enum_v<T>,
    tainted_fundamental_or_enum<false /* TUseAppRep */, T, TSbx>,
    tainted_volatile_standard_pointer<false /* TUseAppRep */, T, TSbx>>;

}  // namespace rlbox
