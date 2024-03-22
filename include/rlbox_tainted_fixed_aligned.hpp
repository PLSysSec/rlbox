/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fixed_aligned type pointer type.
 */

#pragma once

#include <cstddef>
// Needed by included file rlbox_tainted_fixed_aligned.arithmeticop.inc.hpp
#include <stdint.h>  // IWYU pragma: keep
#include <type_traits>
#include <utility>

#include "rlbox_abi_conversion.hpp"
// Needed by included file rlbox_tainted_fixed_aligned.arithmeticop.inc.hpp
#include "rlbox_error_handling.hpp"  // IWYU pragma: keep
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox_plugin_base.hpp"
#include "rlbox_types.hpp"
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
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_impl<
    TUseAppRep, TAppRep, TSbx,
    RLBOX_SPECIALIZE(
        TUseAppRep&& std::is_pointer_v<TAppRep>&& TSbx::mTaintedPointerChoice ==
        tainted_pointer_t::TAINTED_POINTER_FIXED_ALIGNED)>
    : tainted_base<TUseAppRep, TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 protected:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<TAppRep, TSbx>;

  /**
   * @brief The current class's type
   */
  using this_t =
      tainted_impl<TUseAppRep, TAppRep, TSbx,
                   RLBOX_SPECIALIZE(
                       TUseAppRep&& std::is_pointer_v<TAppRep>&&
                           TSbx::mTaintedPointerChoice ==
                       tainted_pointer_t::TAINTED_POINTER_FIXED_ALIGNED)>;

  void dummy_check() {
    static_assert(
        std::is_same_v<this_t, std::remove_pointer_t<decltype(this)>>);
  }

  /**
   * @brief Result type of operator*
   */
  using TOpDeref =
      tainted_volatile<std::remove_pointer_t<detail::tainted_rep_t<TAppRep>>,
                       TSbx>;

  TOpDeref* data{0};

  ////////////////////////////////

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. Use with
   * care as the pointer passed is not checked and is assumed to point to
   * sandbox memory.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   * @return this_t is the tainted pointer of the given param
   */
  static this_t from_unchecked_raw_pointer(TOpDeref* aPtr) {
    this_t ret;
    ret.data = aPtr;
    return ret;
  }

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. Use with
   * care as the pointer passed is not checked and is assumed to point to
   * sandbox memory.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   * @return this_t is the tainted pointer of the given param
   */
  static this_t from_unchecked_raw_pointer(uintptr_t aPtr) {
    // NOLINTNEXTLINE(google-readability-casting)
    return from_unchecked_raw_pointer((TOpDeref*)aPtr);
  }

 public:
  /**
   * @brief Construct a new tainted fixed_aligned_pointer set to null
   */
  inline tainted_impl() = default;
  /**
   * @brief Copy constructor: Construct a new tainted fixed_aligned_pointer
   * object
   */
  inline tainted_impl(const this_t&) = default;
  /**
   * @brief Move constructor: Construct a new tainted fixed_aligned_pointer
   * object
   */
  inline tainted_impl(this_t&&) noexcept = default;

  /**
   * @brief Construct a new tainted fixed_aligned_pointer with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_impl([[maybe_unused]] const std::nullptr_t& aNull) : data(0) {}

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the rhs AppRep
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam TExtraOther... is the extra args of the rhs
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>> &&
          !detail::is_same_wrapper_type_v<this_t, TWrap, TUseAppRepOther,
                                          TAppRepOther, TSbx, TExtraOther...> &&
          std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
                                  TAppRepOther>)>
  inline tainted_impl(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>& aOther)
      : data(aOther.raw_host_rep()) {}

  /**
   * @brief Destroy the tainted fixed_aligned_pointer object
   */
  inline ~tainted_impl() = default;

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified()
      const {
    /// \todo eliminate cast
    return reinterpret_cast<detail::tainted_rep_t<TAppRep>>(data);
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
    /// We need to construct an example_unsandboxed_ptr in order to call @ref
    /// rlbox::rlbox_sandbox::get_sandboxed_pointer_with_example. Since tainted
    /// pointers are already checked to live within the sandbox, the current
    /// pointer itself is a valid example pointer.
    const void* example_unsandboxed_ptr = data;
    auto ret = rlbox_sandbox<TSbx>::get_sandboxed_pointer_with_example(
        data, example_unsandboxed_ptr);
    return ret;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed(
      rlbox_sandbox<TSbx>& aSandbox) const {
    return aSandbox.get_sandboxed_pointer(data);
  }

  ////////////////////////////////

  /**
   * @brief Copy assignment operator
   * @param aOther is the rhs argument
   * @return this_t& returns this object
   */
  inline this_t& operator=(const this_t& aOther) noexcept = default;

  /**
   * @brief Move assignment operator
   * @param aOther is the rhs argument
   * @return this_t& returns this object
   */
  inline this_t& operator=(this_t&& aOther) noexcept = default;

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the rhs AppRep
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam TExtraOther... is the extra args of the rhs
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return this_t& is the reference to this value
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>> &&
          !detail::is_same_wrapper_type_v<this_t, TWrap, TUseAppRepOther,
                                          TAppRepOther, TSbx, TExtraOther...> &&
          std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TAppRepOther>)>
  inline this_t& operator=(const TWrap<TUseAppRepOther, TAppRepOther, TSbx,
                                       TExtraOther...>& aOther) noexcept {
    data = aOther.raw_host_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   */
  inline this_t& operator=(const std::nullptr_t& /* unused */) noexcept {
    data = 0;
    return *this;
  }

  ////////////////////////////////

  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref& operator*() const noexcept { return *data; }

  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @return TOpDeref* is the ptr to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref* operator->() const noexcept {
    // call the deference operator * and then take the address of the result
    // Use std::addressof so we don't call the operator overload of operator &
    return std::addressof(**this);
  }

  /**
   * @brief Operator[] which dereferences a tainted pointer at in idx and gives
   * a tainted_volatile&
   * @param aIdx is the index
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref& operator[](size_t aIdx) {
    const std::remove_pointer_t<decltype(this)> data_idx = *this + aIdx;
    return *data_idx;
  }

  /**
   * @brief Operator[] which dereferences a tainted pointer at in idx and gives
   * a tainted_volatile&
   * @param aIdx is the index
   * @return const TOpDeref& is the const reference to the sandbox memory that
   * holds this data, i.e., memory which is a tainted_volatile type
   */
  inline const TOpDeref& operator[](size_t aIdx) const {
    std::remove_pointer_t<decltype(this)> data_idx = *this + aIdx;
    return *data_idx;
  }

  /**
   * @brief Operator[] which dereferences a tainted pointer at in idx and gives
   * a tainted_volatile&
   * @tparam T is the type of the tainted index
   * @param aIdx is the tainted index
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline TOpDeref& operator[](tainted<T, TSbx> aIdx) {
    return (*this)[aIdx.raw_host_rep()];
  }

  /**
   * @brief Operator[] which dereferences a tainted pointer at in idx and gives
   * a tainted_volatile&
   * @tparam T is the type of the tainted index
   * @param aIdx is the tainted index
   * @return const TOpDeref& is the const reference to the sandbox memory that
   * holds this data, i.e., memory which is a tainted_volatile type
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline const TOpDeref& operator[](tainted<T, TSbx> aIdx) const {
    return (*this)[aIdx.raw_host_rep()];
  }

  ////////////////////////////////

  /**
   * @brief Operator== behaves as expected for tainted pointers and compares the
   * pointer for equality.
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @param aOther is the rhs argument
   * @return if the pointers refer to the same address
   */
  template <typename TAppRepOther>
  friend inline bool operator==(
      const this_t& aThis,
      const tainted_impl<TUseAppRep, TAppRepOther, TSbx>& aOther) noexcept {
    return aThis.data == aOther.data;
  }

  /**
   * @brief Operator!= behaves as expected for tainted pointers and compares the
   * pointer for inequality.
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @param aOther is the rhs argument
   * @return if the pointers refer to different addresses
   */
  template <typename TAppRepOther>
  friend inline bool operator!=(
      const this_t& aThis,
      const tainted_impl<TUseAppRep, TAppRepOther, TSbx>& aOther) noexcept {
    return aThis.data != aOther.data;
  }

 protected:
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
  inline operator bool() const noexcept { return !is_null(); }

  ////////////////////////////////

#define RLBOX_ARITHMETIC_OP +
#define RLBOX_ARITHMETIC_ASSIGN_OP +=
#include "rlbox_tainted_fixed_aligned.arithmeticop.inc.hpp"

#define RLBOX_ARITHMETIC_OP -
#define RLBOX_ARITHMETIC_ASSIGN_OP -=
#include "rlbox_tainted_fixed_aligned.arithmeticop.inc.hpp"
};

}  // namespace rlbox
