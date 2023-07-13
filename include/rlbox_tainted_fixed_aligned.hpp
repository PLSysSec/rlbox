/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
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
template <bool TUseAppRep, typename T, typename TSbx>
class tainted_fixed_aligned_pointer : public tainted_any_base<true, T, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

  static_assert(TUseAppRep, "Expected TUseAppRep to be true");
  static_assert(std::is_pointer_v<T>, "Expected T to be a pointer");

 private:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<T, TSbx>;

  // NOLINTNEXTLINE(hicpp-use-nullptr, modernize-use-nullptr)
  detail::tainted_rep_t<T> data{0};

  ////////////////////////////////

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. This
   * function is for internal use only, as the pointer passed is not checked and
   * is assumed to point to sandbox memory.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   */
  static tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>
  from_unchecked_raw_pointer(T aPtr) {
    tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx> ret;
    ret.data = aPtr;
    return ret;
  }

 public:
  /**
   * @brief Construct a tainted pointer set to null
   */
  inline tainted_fixed_aligned_pointer() = default;

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TWrap is the rhs wrapper type
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
  inline tainted_fixed_aligned_pointer(
      const TWrap<TUseAppRepOther, TOther, TSbx>& aOther)
      : data(aOther.raw_host_rep()) {}

  /**
   * @brief Construct a tainted value with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>(
      const std::nullptr_t& aNull)
      : data(aNull) {}

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::tainted_rep_t<T> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<T> UNSAFE_unverified() const {
    return data;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<T> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<T> UNSAFE_unverified([
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
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the TUseAppRep of the rhs value
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this is (1) a tainted wrapper,
   * (2) meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>& is the
   * reference to this value
   */
  template <template <bool, typename, typename> typename TWrap,
            bool TUseAppRepOther, typename TOther,
            RLBOX_REQUIRE(
                detail::is_tainted_any_wrapper_v<
                    TWrap<TUseAppRepOther, TOther, TSbx>, TSbx>&&
                    std::is_assignable_v<detail::tainted_rep_t<T>&, TOther>)>
  inline tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>& operator=(
      const TWrap<TUseAppRepOther, TOther, TSbx>& aOther) {
    data = aOther.raw_host_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values from a raw primitive value
   * @tparam TOther is the type of the rhs value being assigned
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion. This is allowed for primitive types only.
   * @param aOther is the raw primitive
   * @return tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>& is the
   * reference to this value
   */
  template <
      typename TOther,
      RLBOX_REQUIRE(std::is_assignable_v<detail::tainted_rep_t<T>&, TOther>&&
                        detail::is_fundamental_or_enum_v<T>)>
  inline tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>& operator=(
      const TOther& aOther) {
    data = aOther;
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_fixed_aligned_pointer<TUseAppRep, T, TSbx>& operator=(
      const std::nullptr_t& aNull) {
    data = aNull;
    return *this;
  }

  ////////////////////////////////

 private:
  template <typename TSub>
  using tainted_volatile = typename TSbx::template tainted_volatile<TSub>;

  /**
   * @brief Result type of operator*
   */
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
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
    return *data_tainted_volatile;
  }
};

/**
 * @brief Implementation of a tainted data wrapper that assumes a fixed sandbox
 * heap aligned to its size.
 *
 * @details This is implemented using @ref rlbox::tainted_fundamental_or_enum
 * for fundamental or enum types, @ref rlbox::tainted_fixed_aligned_pointer for
 * pointer types.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
using tainted_fixed_aligned = std::conditional_t<
    detail::is_fundamental_or_enum_v<T>,
    tainted_fundamental_or_enum<true /* TUseAppRep */, T, TSbx>,
    tainted_fixed_aligned_pointer<true /* TUseAppRep */, T, TSbx>>;

}  // namespace rlbox
