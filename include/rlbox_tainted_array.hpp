/**
 * @file rlbox_tainted_array.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_impl that adds
 * tainted/tainted_volatile support for array types.
 */

#pragma once

#include "rlbox_abi_conversion.hpp"
#include "rlbox_error_handling.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_memory_mgmt.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_type_traits.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

#include <array>
#include <stddef.h>
#include <string.h>
#include <type_traits>

namespace rlbox {

/**
 * @brief Implementation of class tht handles both tainted/tainted volatile
 * wrappers for array types
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam TAppRep is the app representation of data
 * @tparam TSbx is the type of sandbox
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_impl<
    TUseAppRep, TAppRep, TSbx,
    RLBOX_SPECIALIZE(
        detail::is_any_array_v<detail::rlbox_stdint_to_stdint_t<TAppRep>>)> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 protected:
  /**
   * @brief The current class's type
   */
  using this_t = tainted_impl<
      TUseAppRep, TAppRep, TSbx,
      RLBOX_SPECIALIZE(
          detail::is_any_array_v<detail::rlbox_stdint_to_stdint_t<TAppRep>>)>;

  void dummy_check() {
    static_assert(
        std::is_same_v<this_t, std::remove_pointer_t<decltype(this)>>);
  }

  using TArrEl = std::remove_extent_t<detail::std_array_to_c_array_t<TAppRep>>;

  /// \todo Make this wrapper work with n dimensional arrays
  static_assert(!detail::is_any_array_v<TArrEl>,
                "Multidimentional arrays not yet supported in tainted");

  using TAppRepEl = detail::tainted_rep_t<TArrEl>;

  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRepEl =
      detail::tainted_rep_t<detail::rlbox_base_types_convertor<TArrEl, TSbx>>;

  using TRepEl = tainted_impl<TUseAppRep, TArrEl, TSbx>;
  static constexpr size_t TRepElCount =
      std::extent_v<detail::std_array_to_c_array_t<TAppRep>>;

 public:
  /**
   * @brief Represent a tainted array as an array of tainted values
   */
  std::array<TRepEl, TRepElCount> data{{0}};

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return std::array<TAppRepEl, TRepElCount> is the raw data
   */
  [[nodiscard]] inline std::array<TAppRepEl, TRepElCount> UNSAFE_unverified(
      rlbox_sandbox<TSbx>& aSandbox) const {
    std::array<TAppRepEl, TRepElCount> converted;
    /// \todo Replace with rlbox::memcpy
    for (size_t i = 0; i < TRepElCount; i++) {
      auto untainted_val = data[i].UNSAFE_unverified(aSandbox);
      memcpy(&(converted[i]), &untainted_val, sizeof(untainted_val));
    }
    return converted;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return std::array<TSbxRepEl, TRepElCount> is the raw data in the
   * sandboxed ABI
   */
  [[nodiscard]] inline std::array<TSbxRepEl, TRepElCount> UNSAFE_sandboxed(
      rlbox_sandbox<TSbx>& aSandbox) const {
    std::array<TSbxRepEl, TRepElCount> converted;
    /// \todo Replace with rlbox::memcpy
    for (size_t i = 0; i < TRepElCount; i++) {
      auto untainted_val = data[i].UNSAFE_sandboxed(aSandbox);
      memcpy(&(converted[i]), &untainted_val, sizeof(untainted_val));
    }
    return converted;
  }

  ////////////////////////////////

  ///\todo Modify noexcept with simple macro on EXCEPTIONS_ENABLED

  /**
   * @brief Operator[] which dereferences a tainted array at an idx and gives a
   * tainted&
   * @param aIdx is the index
   * @return TRepEl& is the reference to the tainted element
   */
  inline TRepEl& operator[](size_t aIdx) noexcept(
      noexcept(detail::dynamic_check(false, ""))) {
    detail::dynamic_check(aIdx < TRepElCount,
                          "Out of bounds access to a tainted array");
    return data[aIdx];
  }

  /**
   * @brief Operator[] which dereferences a tainted array at an idx and gives a
   * tainted&
   * @tparam TWrap is the index wrapper type
   * @tparam TUseAppRepOther is the index AppRep
   * @tparam TAppRepOther is the type of the index value being wrapped
   * @tparam TExtraOther... is the extra args of the index
   * @param aIdx is the tainted index
   * @return TRepEl& is the reference to the tainted element
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<TWrap<
                        TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>>)>
  inline TRepEl& operator[](
      TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>
          aIdx) noexcept(noexcept(detail::dynamic_check(false, ""))) {
    auto idx_untainted = aIdx.raw_host_rep();
    if constexpr (std::is_signed_v<decltype(idx_untainted)>) {
      detail::dynamic_check(idx_untainted >= 0,
                            "Out of bounds access to a tainted array");
    }
    using unsigned_index_t = std::make_unsigned_t<decltype(idx_untainted)>;
    detail::dynamic_check(
        static_cast<unsigned_index_t>(idx_untainted) < TRepElCount,
        "Out of bounds access to a tainted array");
    return data[idx_untainted];
  }

  /**
   * @brief Operator[] const which dereferences a tainted array at an idx and
   * gives a tainted&
   * @param aIdx is the index
   * @return const TRepEl& is the const reference to the the tainted element
   */
  inline const TRepEl& operator[](size_t aIdx) const
      noexcept(noexcept(detail::dynamic_check(false, ""))) {
    detail::dynamic_check(aIdx < TRepElCount,
                          "Out of bounds access to a tainted array");
    return data[aIdx];
  }

  /**
   * @brief Operator[] const which dereferences a tainted array at an idx and
   * gives a tainted&
   * @tparam TWrap is the index wrapper type
   * @tparam TUseAppRepOther is the index AppRep
   * @tparam TAppRepOther is the type of the index value being wrapped
   * @tparam TExtraOther... is the extra args of the index
   * @param aIdx is the tainted index
   * @return TRepEl& is the reference to the tainted element
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<TWrap<
                        TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>>)>
  inline const TRepEl& operator[](
      TWrap<TUseAppRepOther, TAppRepOther, TExtraOther...> aIdx) const
      noexcept(noexcept(detail::dynamic_check(false, ""))) {
    auto idx_untainted = aIdx.raw_host_rep();
    if constexpr (std::is_signed_v<decltype(idx_untainted)>) {
      detail::dynamic_check(idx_untainted >= 0,
                            "Out of bounds access to a tainted array");
    }
    using unsigned_index_t = std::make_unsigned_t<decltype(idx_untainted)>;
    detail::dynamic_check(
        static_cast<unsigned_index_t>(idx_untainted) < TRepElCount,
        "Out of bounds access to a tainted array");
    return data[idx_untainted];
  }

  /**
   * @brief Operator* which dereferences tainted and gives a tainted array
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TRepEl& operator*() const noexcept(noexcept(data[0])) {
    return data[0];
  }

 private:
  using TToPointerRet =
      std::conditional_t<TUseAppRep, rlbox_unique_ptr<TAppRepEl, TSbx>,
                         tainted<TAppRepEl*, TSbx>>;

 public:
  inline TToPointerRet to_pointer(rlbox_sandbox<TSbx>& aSandbox) {
    if constexpr (!TUseAppRep) {
      // the array is already in the sandbox, simply return the address
      TRepEl* arr_ptr = data.data();
      return TToPointerRet::from_unchecked_raw_pointer(arr_ptr);
    } else {
      constexpr size_t el_count = sizeof(TAppRep) / sizeof(TAppRepEl);
      /// \todo Ensure this works without the tainted cast
      TToPointerRet buff = make_unique_tainted_many<TAppRepEl>(
          aSandbox, tainted<size_t, TSbx>(el_count));
      /// \todo Replace with rlbox::memcpy
      for (size_t i = 0; i < el_count; i++) {
        buff[i] = data[i];
      }
      return buff;
    }
  }
};

}  // namespace rlbox
