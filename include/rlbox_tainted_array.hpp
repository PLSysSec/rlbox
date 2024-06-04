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
#include "rlbox_type_traits.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

#include <array>
#include <stddef.h>
#include <type_traits>

namespace rlbox {

/**
 * @brief Implementation of class tht handles both tainted/tainted volatile
 * wrappers for array types
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam TAppRep is the app representation of data
 * @tparam TSbxRep is the sandbox representation of data
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

  /**
   * @brief The sandbox representation of data for this wrapper, with std::array
   * converted to a regular array.
   */
  using TAppRepFixed = detail::std_array_to_c_array_t<TAppRep>;

  using TAppRepEl =
      tainted_impl<TUseAppRep, std::remove_extent_t<TAppRepFixed>, TSbx>;

  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::tainted_rep_t<
      detail::rlbox_base_types_convertor<TAppRepFixed, TSbx>>;

  using TSbxRepEl =
      tainted_impl<TUseAppRep, std::remove_extent_t<TSbxRep>, TSbx>;

  /**
   * @brief The internal representation of data for this wrapper
   */
  using TRep = detail::std_array_to_c_array_t<
      std::conditional_t<TUseAppRep, detail::tainted_rep_t<TAppRepFixed>,
                         detail::tainted_rep_t<TSbxRep>>>;

  using TRepEl = tainted_impl<TUseAppRep, std::remove_extent_t<TRep>, TSbx>;

 public:
  /**
   * @brief Represent a tainted array as an array of tainted values
   */
  std::array<TRepEl, std::extent_v<TRep>> data{{0}};

  // /**
  //  * @brief Construct a fundamental tainted_impl object
  //  */
  // inline tainted_impl() = default;
  // /**
  //  * @brief Copy constructor: Construct a fundamental tainted_impl
  //  * object
  //  */
  // inline tainted_impl(const this_t&) = default;
  // /**
  //  * @brief Move constructor: Construct a fundamental tainted_impl object
  //  */
  // inline tainted_impl(this_t&&) noexcept = default;

  // /**
  //  * @brief Construct a new tainted object from another tainted wrapped
  //  object
  //  * @tparam TWrap is the rhs wrapper type
  //  * @tparam TUseAppRepOther is the rhs AppRep
  //  * @tparam TAppRepOther is the type of the rhs value being wrapped
  //  * @tparam TExtraOther... is the extra args of the rhs
  //  * @tparam RLBOX_REQUIRE checks if (1) this won't be handled the original
  //  * class's copy/move constructor and (2) this meets the constructible
  //  * criterion
  //  * @param aOther is the rhs being assigned
  //  */
  // template <
  //     template <bool, typename, typename, typename...> typename TWrap,
  //     bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
  //     RLBOX_REQUIRE(
  //         detail::is_tainted_any_wrapper_v<
  //             TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>> &&
  //         !detail::is_same_wrapper_type_v<this_t, TWrap, TUseAppRepOther,
  //                                         TAppRepOther, TSbx, TExtraOther...>
  //                                         &&
  //         std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
  //                                 TAppRepOther>)>
  // inline tainted_impl(
  //     const TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>&
  //     aOther) : data([&] {
  //         if constexpr (TUseAppRep) {
  //           return aOther.raw_host_rep();
  //         } else {
  //           return aOther.raw_sandbox_rep();
  //         }
  //       }()) {}

  // /**
  //  * @brief Construct a new tainted object from a raw (unwrapped) primitive
  //  * value
  //  * @tparam TOther is the type of the rhs value
  //  * @tparam RLBOX_REQUIRE checks if this meets the constructible criterion
  //  * @param aOther is the raw primitive
  //  */
  // template <typename... TArgs,
  //           RLBOX_REQUIRE(std::is_constructible_v<
  //                         decltype(data), TArgs&&...>)>
  // inline tainted_impl(TArgs&&... aArgs)
  //     : data(std::forward<TArgs>(aArgs)...) {}

  /**
   * @brief Destroy the fundamental tainted_impl object
   */
  // inline ~tainted_impl() = default;

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return std::array<TAppRepEl, std::extent_v<TRep>> is the raw data
   */
  [[nodiscard]] inline std::array<TAppRepEl, std::extent_v<TRep>>
  UNSAFE_unverified(rlbox_sandbox<TSbx>& aSandbox) const {
    if constexpr (TUseAppRep) {
      return data;
    } else {
      std::array<TAppRepEl, std::extent_v<TRep>> converted;
      for (size_t i = 0; i < std::extent_v<TRep>; i++) {
        auto untainted_val = data[i].UNSAFE_unverified(aSandbox);
        memcpy(&(converted[i]), &untainted_val, sizeof(untainted_val));
      }
      return converted;
    }
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return std::array<TSbxRepEl, std::extent_v<TRep>> is the raw data in the
   * sandboxed ABI
   */
  [[nodiscard]] inline std::array<TSbxRepEl, std::extent_v<TRep>>
  UNSAFE_sandboxed(rlbox_sandbox<TSbx>& aSandbox) const {
    if constexpr (TUseAppRep) {
      std::array<TSbxRepEl, std::extent_v<TRep>> converted;
      for (size_t i = 0; i < std::extent_v<TRep>; i++) {
        auto untainted_val = data[i].UNSAFE_sandboxed(aSandbox);
        memcpy(&(converted[i]), &untainted_val, sizeof(untainted_val));
      }
      return converted;
    } else {
      return data;
    }
  }

  ////////////////////////////////

  ///\todo Modify noexcept with simple macro on EXCEPTIONS_ENABLED

  /**
   * @brief Operator[] which dereferences a tainted array at in idx and gives a
   * tainted&
   * @param aIdx is the index
   * @return TRepEl& is the reference to the tainted element
   */
  inline TRepEl& operator[](size_t aIdx) noexcept(
      noexcept(detail::dynamic_check(false, ""))) {
    detail::dynamic_check(aIdx < std::extent_v<TAppRepFixed>,
                          "Out of bounds access to a tainted array");
    return data[aIdx];
  }

  /**
   * @brief Operator[] which dereferences a tainted array at in idx and gives a
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
      TWrap<TUseAppRepOther, TAppRepOther, TExtraOther...>
          aIdx) noexcept(noexcept(detail::dynamic_check(false, ""))) {
    auto idx_untainted = aIdx.raw_host_rep();
    if constexpr (std::is_signed_v<decltype(idx_untainted)>) {
      detail::dynamic_check(idx_untainted >= 0,
                            "Out of bounds access to a tainted array");
    }
    detail::dynamic_check(idx_untainted < std::extent_v<TAppRepFixed>,
                          "Out of bounds access to a tainted array");
    return data[idx_untainted];
  }

  /**
   * @brief Operator[] const which dereferences a tainted array at in idx and
   * gives a tainted&
   * @param aIdx is the index
   * @return const TRepEl& is the const reference to the the tainted element
   */
  inline const TRepEl& operator[](size_t aIdx) const
      noexcept(noexcept(detail::dynamic_check(false, ""))) {
    detail::dynamic_check(aIdx < std::extent_v<TAppRepFixed>,
                          "Out of bounds access to a tainted array");
    return data[aIdx];
  }

  /**
   * @brief Operator[] const which dereferences a tainted array at in idx and
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
    detail::dynamic_check(idx_untainted < std::extent_v<TAppRepFixed>,
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
};

}  // namespace rlbox
