/**
 * @file rlbox_tainted_relocatable.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_relocatable pointer type.
 */

#pragma once

#include <cstdlib>
// IWYU incorrectly reports this as unnecessary as the use of type_traits is in
// a templated class
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox_plugin_base.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief Implementation of tainted data wrappers for pointer data that supports
 * the movement of the sandbox heap after creation.
 * @details The pointer value is stored as a pointer to the heap base (which can
 * be updated) plus an offset from the heap base.
 * @tparam TData is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TData, typename TSbx>
class tainted_impl<
    TUseAppRep, TData, TSbx,
    RLBOX_SPECIALIZE(
        TUseAppRep&& std::is_pointer_v<TData>&& TSbx::mTaintedPointerChoice ==
        tainted_pointer_t::TAINTED_POINTER_RELOCATABLE)> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 protected:
  /**
   * @brief The app representation of data for this wrapper
   */
  using TAppRep = detail::tainted_rep_t<TData>;

  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep =
      detail::tainted_rep_t<detail::rlbox_base_types_convertor<TData, TSbx>>;

  /**
   * @brief The current class's type
   */
  using this_t = tainted_impl<
      TUseAppRep, TData, TSbx,
      RLBOX_SPECIALIZE(
          TUseAppRep&& std::is_pointer_v<TData>&& TSbx::mTaintedPointerChoice ==
          tainted_pointer_t::TAINTED_POINTER_RELOCATABLE)>;

  void dummy_check() {
    static_assert(
        std::is_same_v<this_t, std::remove_pointer_t<decltype(this)>>);
  }

  /**
   * @brief The internal representation of data for this wrapper
   */
  using TRep = detail::get_equivalent_uint_t<typename TSbx::sbx_pointer>;

  TRep data{0};

 public:
  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return TAppRep is the raw data
   */
  [[nodiscard]] inline TAppRep UNSAFE_unverified() const {
    return std::abort();
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @return TSbxRep is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline TSbxRep UNSAFE_sandboxed() const { return std::abort(); }
};

}  // namespace rlbox
