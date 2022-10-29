/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fixed_aligned class.
 */

#pragma once

#include <stddef.h>
#include <type_traits>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_type_conversion.hpp"
#include "rlbox_type_traits.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {
/**
 * @brief Implementation of tainted data wrappers that requires a fixed sandbox
 * heap aligned to its size.
 *
 * @details Tainted data wrappers (wrappers that identify data returned by
 * sandboxed code) that requires:
 * - the sandbox heap cannot be moved once created (fixed). The heap maybe grown
 * in-place.
 * - the sandbox heap is aligned to its size (aligned), even on growth.
 *      E.g., a heap of max size 4gb is aligned to 4gb
 *
 * Due to these assumptions, if the tainted data is a pointer type, the value is
 * stored as a global pointer.
 *
 * @note This implementatation handles only non-class/non-struct T types.
 * Specializations of this class are generated by the rlbox_lib_load_classes
 * when T is a class/struct.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_fixed_aligned : public tainted_base<T, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 private:
  using TSbxRep = detail::rlbox_base_types_convertor<T, TSbx>;

  // NOLINTNEXTLINE(hicpp-use-nullptr, modernize-use-nullptr)
  detail::tainted_rep_t<T> data{0};

  ////////////////////////////////

  /**
   * @brief Construct a tainted value with a pointer to sandbox memory. This
   * function is for internal use only, as the pointer passed is not checked and
   * is assumed to point to sandbox memory.
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE ensures this is allowed for pointer types only.
   * @param aPtr is a pointer that is assumed to point to an object in sandbox
   * memory.
   */
  template <typename TDummy = T, RLBOX_REQUIRE(std::is_pointer_v<TDummy>)>
  static tainted_fixed_aligned<T, TSbx> from_unchecked_raw_pointer(T aPtr) {
    tainted_fixed_aligned ret;
    ret.data = aPtr;
    return ret;
  }

 public:
  inline tainted_fixed_aligned() = default;

  /**
   * @brief Construct a new tainted object from another tainted wrapped object
   * @tparam TWrap is the other wrapper type
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this is (1) a tainted wrapper,
   * (2) meets the assignable criterion
   * @param aOther is the rhs being assigned
   */
  template <
      template <typename, typename...> typename TWrap, typename TOther,
      RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<TWrap, TOther, TSbx>&&
                        std::is_assignable_v<decltype(data)&, TOther>)>
  inline tainted_fixed_aligned(const TWrap<TOther, TSbx>& aOther)
      : data(aOther.raw_host_rep()) {}

  /**
   * @brief Construct a new tainted object from a raw primitive value
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE param ensures this is allowed for primitive types
   * only.
   * @param aOther is the raw primitive
   */
  template <typename TDummy = T,
            RLBOX_REQUIRE(detail::is_fundamental_or_enum_v<TDummy>)>
  inline tainted_fixed_aligned(const T& aOther) : data(aOther) {}

  /**
   * @brief Construct a tainted value with a nullptr
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE ensures this is allowed for pointer types only.
   * @param aNull is a nullptr
   */
  template <typename TDummy = T, RLBOX_REQUIRE(std::is_pointer_v<TDummy>)>
  inline tainted_fixed_aligned<T, TSbx>(const std::nullptr_t& aNull)
      : data(aNull) {}

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::value_type_t<T> is the raw data
   */
  [[nodiscard]] inline detail::value_type_t<T> UNSAFE_unverified() const {
    return data;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::value_type_t<T> is the raw data
   */
  [[nodiscard]] inline detail::value_type_t<T> UNSAFE_unverified([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_unverified();
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI. This is not supported for pointer types.
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE param ensures this is allowed for primitive types
   * only.
   * @return detail::value_type_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  template <typename TDummy = T,
            RLBOX_REQUIRE(detail::is_fundamental_or_enum_v<TDummy>)>
  [[nodiscard]] inline detail::value_type_t<TSbxRep> UNSAFE_sandboxed() const {
    auto converted =
        detail::convert_type_fundamental<detail::value_type_t<TSbxRep>>(data);
    return converted;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::value_type_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::value_type_t<TSbxRep> UNSAFE_sandboxed([
      [maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    if constexpr (std::is_pointer_v<T>) {
      return aSandbox.get_sandboxed_pointer(data);
    } else {
      return UNSAFE_sandboxed();
    }
  }

  ////////////////////////////////

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the other wrapper type
   * @tparam TOther is the type of the rhs value being wrapped
   * @tparam RLBOX_REQUIRE param checks to see if this is (1) a tainted wrapper,
   * (2) meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return tainted_fixed_aligned<T, TSbx>& is the reference to this value
   */
  template <
      template <typename, typename> typename TWrap, typename TOther,
      RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<TWrap, TOther, TSbx>&&
                        std::is_assignable_v<decltype(data)&, TOther>)>
  inline tainted_fixed_aligned<T, TSbx>& operator=(
      const TWrap<TOther, TSbx>& aOther) {
    data = aOther.raw_host_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values from a raw primitive value
   * @tparam TOther is the type of the rhs value being assigned
   * @tparam RLBOX_REQUIRE param checks to see if this meets the assignable
   * criterion. This is allowed for primitive types only.
   * @param aOther is the raw primitive
   * @return tainted_fixed_aligned<T, TSbx>& is the reference to this value
   */
  template <typename TOther,
            RLBOX_REQUIRE(std::is_assignable_v<decltype(data)&, TOther>&&
                              detail::is_fundamental_or_enum_v<T>)>
  inline tainted_fixed_aligned<T, TSbx>& operator=(const TOther& aOther) {
    data = aOther;
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   * @tparam TDummy is a dummy parameter to do our static type checks
   * @tparam RLBOX_REQUIRE ensures this is allowed for pointer types only.
   * @param aNull is a nullptr
   */
  template <typename TDummy = T, RLBOX_REQUIRE(std::is_pointer_v<TDummy>)>
  inline tainted_fixed_aligned<T, TSbx>& operator=(
      const std::nullptr_t& aNull) {
    data = aNull;
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
  template <typename TDummy = T, RLBOX_REQUIRE(std::is_pointer_v<TDummy>)>
  inline TOpDeref& operator*() {
    // Deliberately use a C style cast as we we want to get rid of any CV
    // qualifers here. CV qualifiers are moved inside the wrapper type and thus
    // continue to be tracked.

    // NOLINTNEXTLINE(google-readability-casting)
    auto data_tainted_volatile = (TOpDeref*)data;
    return *data_tainted_volatile;
  }
};

}  // namespace rlbox