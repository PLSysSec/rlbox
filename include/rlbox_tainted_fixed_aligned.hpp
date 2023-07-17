/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fixed_aligned type.
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
 * @brief Implementation of a tainted data wrapper for pointer data that assumes
 * a fixed sandbox heap aligned to its size.
 *
 * @details Tainted data wrappers (wrappers that identify data returned by
 * sandboxed code) that requires:
 * - the sandbox heap cannot be moved once created (fixed). The heap maybe grown
 *   in-place.
 * - the sandbox heap is aligned to its size (aligned), even on growth. E.g., a
 *      heap of max size 4gb is aligned to 4gb
 *
 * Due to these assumptions, the pointer value can be stored as a global
 * pointer.
 *
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_fixed_aligned_pointer
    : public tainted_any_base<true, TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

  static_assert(TUseAppRep, "Expected TUseAppRep to be true");
  static_assert(std::is_pointer_v<TAppRep>, "Expected TAppRep to be a pointer");

 private:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<TAppRep, TSbx>;

  detail::tainted_rep_t<TAppRep> data{0};

  ////////////////////////////////

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. This
   * function is for internal use only, as the pointer passed is not checked and
   * is assumed to point to sandbox memory.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   */
  static tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>
  from_unchecked_raw_pointer(TAppRep aPtr) {
    tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx> ret;
    ret.data = aPtr;
    return ret;
  }

 public:
  /**
   * @brief Construct a tainted_fixed_aligned_pointer set to null
   */
  inline tainted_fixed_aligned_pointer() = default;
  /**
   * @brief Copy constructor: Construct a new tainted_fixed_aligned_pointer
   * object
   */
  inline tainted_fixed_aligned_pointer(
      const tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>&) =
      default;
  /**
   * @brief Move constructor: Construct a new tainted_fixed_aligned_pointer
   * object
   */
  inline tainted_fixed_aligned_pointer(
      tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>&&) noexcept =
      default;

  /**
   * @brief Construct a tainted_fixed_aligned_pointer with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_fixed_aligned_pointer([
      [maybe_unused]] const std::nullptr_t& aNull)
      : data(0) {}

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <template <bool, typename, typename> typename TWrap,
            bool TUseAppRepOther, typename TAppRepOther,
            RLBOX_REQUIRE(
                !std::is_same_v<
                    tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>,
                    tainted_fixed_aligned_pointer<TUseAppRepOther, TAppRepOther,
                                                  TSbx>> &&

                detail::is_tainted_any_wrapper_v<
                    TWrap<TUseAppRepOther, TAppRepOther, TSbx>> &&
                std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
                                        TAppRepOther>)>
  inline tainted_fixed_aligned_pointer(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx>& aOther)
      : data(aOther.raw_host_rep()) {}

  /**
   * @brief Destroy the tainted_fixed_aligned_pointer object
   */
  inline ~tainted_fixed_aligned_pointer() = default;

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified()
      const {
    return data;
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
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return aSandbox.get_sandboxed_pointer(data);
  }

  ////////////////////////////////

  /**
   * @brief Copy assignment operator
   * @param aOther is the rhs argument
   * @return tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>&
   * returns this object
   */
  inline tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& operator=(
      const tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& aOther) =
      default;

  /**
   * @brief Move assignment operator
   * @param aOther is the rhs argument
   * @return tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>&
   * returns this object
   */
  inline tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& operator=(
      tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>&&
          aOther) noexcept = default;

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& is the
   * reference to this value
   */
  template <
      template <bool, typename, typename> typename TWrap, bool TUseAppRepOther,
      typename TAppRepOther,
      RLBOX_REQUIRE(
          !std::is_same_v<
              tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>,
              tainted_fixed_aligned_pointer<TUseAppRepOther, TAppRepOther,
                                            TSbx>> &&
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx>> &&
          std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TAppRepOther>)>
  inline tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& operator=(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx>& aOther) {
    data = aOther.raw_host_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   */
  inline tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>& operator=(
      const std::nullptr_t& /* unused */) noexcept {
    data = 0;
    return *this;
  }

  ////////////////////////////////

 private:
  template <typename TSub>
  using tainted_volatile = typename TSbx::template tainted_volatile<TSub>;

  /**
   * @brief Result type of operator*
   */
  using TOpDeref = tainted_volatile<std::remove_pointer_t<TAppRep>>;

 public:
  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE ensures this is allowed for pointer types only.
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref& operator*() const noexcept {
    /// \todo eliminate cast and replace with tainted_volatile constructor
    /// taking a reference

    auto data_tainted_volatile = reinterpret_cast<TOpDeref*>(data);
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
    return *data_tainted_volatile;
  }

  /**
   * @brief Operator== behaves as expected for tainted pointers and compares the
   * pointer for equality.
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @param aOther is the rhs argument
   * @return if the pointers refer to the same address
   */
  template <typename TAppRepOther>
  inline bool operator==(
      const tainted_fixed_aligned_pointer<TUseAppRep, TAppRepOther, TSbx>&
          aOther) const noexcept {
    return data == aOther.data;
  }
  /**
   * @brief Operator!= behaves as expected for tainted pointers and compares the
   * pointer for inequality.
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @param aOther is the rhs argument
   * @return if the pointers refer to different addresses
   */
  template <typename TAppRepOther>
  inline bool operator!=(
      const tainted_fixed_aligned_pointer<TUseAppRep, TAppRepOther, TSbx>&
          aOther) const noexcept {
    return !((*this) == aOther);
  }

 private:
  /**
   * @brief Helper function that checks if this pointer refers to null
   * @return true if the pointer is null
   */
  [[nodiscard]] inline bool is_null() const noexcept { return data == 0; }

 public:
  /**
   * @brief Check if pointer is null
   */
  inline bool operator==(const std::nullptr_t& /* unused */) const noexcept {
    return is_null();
  }
  /**
   * @brief Check if pointer is not null
   */
  inline bool operator!=(const std::nullptr_t& /* unused */) const noexcept {
    return !is_null();
  }
  /**
   * @brief Convert the pointer to bool.
   * @return true if the pointer is not null
   */
  explicit inline operator bool() const noexcept { return !is_null(); }
};

/**
 * @brief Implementation of a tainted data wrapper that assumes a fixed sandbox
 * heap aligned to its size.
 *
 * @details This is implemented using @ref rlbox::tainted_fundamental_or_enum
 * for fundamental or enum types, @ref rlbox::tainted_fixed_aligned_pointer for
 * pointer types.
 *
 * @tparam TAppRep is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename TAppRep, typename TSbx>
using tainted_fixed_aligned = std::conditional_t<
    detail::is_fundamental_or_enum_v<detail::tainted_rep_t<TAppRep>>,
    tainted_fundamental_or_enum<true /* TUseAppRep */, TAppRep, TSbx>,
    tainted_fixed_aligned_pointer<true /* TUseAppRep */, TAppRep, TSbx>>;

}  // namespace rlbox
