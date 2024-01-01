/**
 * @file rlbox_tainted_fixed_aligned_pointer.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fixed_aligned type pointer type.
 */

#pragma once

#include <cstddef>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_error_handling.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_tainted_base.hpp"
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
template <typename TAppRep, typename TSbx>
class tainted_fixed_aligned_pointer : public tainted_any_base<TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

  static_assert(std::is_pointer_v<TAppRep>, "Expected TAppRep to be a pointer");

 protected:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<TAppRep, TSbx>;

  /**
   * @brief The current class's type
   */
  using this_t = tainted_fixed_aligned_pointer<TAppRep, TSbx>;

  /**
   * @brief tainted type of the sandbox
   */
  template <typename T>
  using tainted = typename TSbx::template tainted<T>;

  /**
   * @brief tainted_volatile type of the sandbox
   */
  template <typename T>
  using tainted_volatile = typename TSbx::template tainted_volatile<T>;

  detail::tainted_rep_t<TAppRep> data{0};

  ////////////////////////////////

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. This
   * function is for internal use only, as the pointer passed is not checked and
   * is assumed to point to sandbox memory.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   * @return this_t is the tainted pointer of the given param
   */
  static this_t from_unchecked_raw_pointer(TAppRep aPtr) {
    this_t ret;
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
  inline tainted_fixed_aligned_pointer(const this_t&) = default;
  /**
   * @brief Move constructor: Construct a new tainted_fixed_aligned_pointer
   * object
   */
  inline tainted_fixed_aligned_pointer(this_t&&) noexcept = default;

  /**
   * @brief Construct a tainted_fixed_aligned_pointer with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_fixed_aligned_pointer(
      [[maybe_unused]] const std::nullptr_t& aNull)
      : data(0) {}

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TWrap is the rhs wrapper type
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <
      template <typename, typename> typename TWrap, typename TAppRepOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<TWrap<TAppRepOther, TSbx>> &&
          !detail::is_same_wrapper_type_v<TWrap, TAppRepOther, TSbx, this_t> &&
          std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
                                  TAppRepOther>)>
  inline tainted_fixed_aligned_pointer(const TWrap<TAppRepOther, TSbx>& aOther)
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
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return this_t& is the reference to this value
   */
  template <
      template <typename, typename> typename TWrap, typename TAppRepOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<TWrap<TAppRepOther, TSbx>> &&
          !detail::is_same_wrapper_type_v<TWrap, TAppRepOther, TSbx, this_t> &&
          std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TAppRepOther>)>
  inline this_t& operator=(const TWrap<TAppRepOther, TSbx>& aOther) noexcept {
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

 protected:
  /**
   * @brief Result type of operator*
   */
  using TOpDeref = tainted_volatile<std::remove_pointer_t<TAppRep>>;

 public:
  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
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
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @return TOpDeref* is the ptr to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref* operator->() const noexcept {
    // call the deference operator * and then take the address of the result
    // Use std::adress of so we don't call the operator overload of operator &
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
  inline TOpDeref& operator[](tainted<T> aIdx) {
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
  inline const TOpDeref& operator[](tainted<T> aIdx) const {
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
  inline bool operator==(const this_t& aOther) const noexcept {
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
  inline bool operator!=(const this_t& aOther) const noexcept {
    return !((*this) == aOther);
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
  explicit inline operator bool() const noexcept { return !is_null(); }

  ////////////////////////////////

  /**
   * @brief Operator+ which increments a tainted pointer by aInc
   * @param aInc is the increment amount
   * @return this_t is the incremented tainted pointer
   */
  inline this_t operator+(size_t aInc) const {
    detail::dynamic_check(!is_null(), "Deferencing a tainted null pointer");
    auto new_data_int =
        reinterpret_cast<uintptr_t>(data) + sizeof(TOpDeref) * aInc;
    auto new_data = reinterpret_cast<decltype(data)>(new_data_int);
    const bool in_bounds =
        rlbox_sandbox<TSbx>::is_pointer_in_sandbox_memory_with_example(new_data,
                                                                       data);
    detail::dynamic_check(in_bounds, "Pointer offset not in sandbox");

    auto ret = this_t::from_unchecked_raw_pointer(new_data);
    return ret;
  }

  /**
   * @brief Operator+ which increments a tainted pointer by aInc
   * @tparam T is the type of the tainted index
   * @param aInc is the increment amount
   * @return this_t is the incremented tainted pointer
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t operator+(tainted<T> aInc) const {
    return (*this) + aInc.raw_host_rep();
  }

  /**
   * @brief Operator+= which increments a tainted pointer by aInc and sets the
   * pointer
   * @param aInc is the increment amount
   * @return this_t& returns this object after modification
   */
  inline this_t& operator+=(size_t aInc) {
    this_t new_ptr = *this + aInc;
    *this = new_ptr;
    return *this;
  }

  /**
   * @brief Operator+= which increments a tainted pointer by aInc and sets the
   * pointer
   * @tparam T is the type of the tainted index
   * @param aInc is the increment amount
   * @return this_t& returns this object after modification
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t& operator+=(tainted<T> aInc) {
    (*this) += aInc.raw_host_rep();
    return *this;
  }

  /**
   * @brief Operator- which decrements a tainted pointer by aInc
   * @param aInc is the decrement amount
   * @return this_t is the decremented tainted pointer
   */
  inline this_t operator-(size_t aInc) const {
    detail::dynamic_check(!is_null(), "Deferencing a tainted null pointer");
    auto new_data_int =
        reinterpret_cast<uintptr_t>(data) - sizeof(TOpDeref) * aInc;
    auto new_data = reinterpret_cast<decltype(data)>(new_data_int);
    bool in_bounds =
        rlbox_sandbox<TSbx>::is_pointer_in_sandbox_memory_with_example(new_data,
                                                                       data);
    detail::dynamic_check(in_bounds, "Pointer offset not in sandbox");

    auto ret = this_t::from_unchecked_raw_pointer(new_data);
    return ret;
  }

  /**
   * @brief Operator- which decrements a tainted pointer by aInc
   * @tparam T is the type of the tainted index
   * @param aInc is the decrement amount
   * @return this_t is the decremented tainted pointer
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t operator-(tainted<T> aInc) const {
    return (*this) - aInc.raw_host_rep();
  }

  /**
   * @brief Operator-= which decrements a tainted pointer by aInc and sets the
   * pointer
   * @param aInc is the decrement amount
   * @return this_t& returns this object after modification
   */
  inline this_t& operator-=(size_t aInc) {
    this_t new_ptr = *this - aInc;
    *this = new_ptr;
    return *this;
  }

  /**
   * @brief Operator-= which decrements a tainted pointer by aInc and sets the
   * pointer
   * @tparam T is the type of the tainted index
   * @param aInc is the decrement amount
   * @return this_t& returns this object after modification
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t& operator-=(tainted<T> aInc) {
    (*this) -= aInc.raw_host_rep();
    return *this;
  }
};

}  // namespace rlbox
