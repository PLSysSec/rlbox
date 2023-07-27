/**
 * @file rlbox_tainted_relocatable.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_relocatable type.
 */

#pragma once

#include <stdlib.h>
// IWYU incorrectly reports this as unnecessary as the use of type_traits is in
// a templated class
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_tainted_fundamental_or_enum.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief Implementation of tainted data wrappers for pointer data that supports
 * the movement of the sandbox heap after creation.
 * @details The pointer value is stored as a pointer to the heap base (which can
 * be updated) plus an offset from the heap base.
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam TAppRep is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_relocatable_pointer
    : public tainted_any_base<TUseAppRep, TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

  static_assert(TUseAppRep, "Expected TUseAppRep to be true");
  static_assert(std::is_pointer_v<TAppRep>, "Expected TAppRep to be a pointer");

 protected:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<TAppRep, TSbx>;

  detail::tainted_rep_t<TSbxRep> data{0};

 public:
  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified()
      const {
    return abort();
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed() const {
    return abort();
  }
};

/**
 * @brief Implementation of tainted data wrappers that supports the movement of
 * the sandbox heap after creation.
 *
 * @details This is implemented using @ref rlbox::tainted_fundamental_or_enum
 * for fundamental or enum types, @ref rlbox::tainted_relocatable_pointer for
 * pointer types.
 *
 * @tparam TAppRep is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename TAppRep, typename TSbx>
using tainted_relocatable = std::conditional_t<
    detail::is_fundamental_or_enum_v<detail::tainted_rep_t<TAppRep>>,
    tainted_fundamental_or_enum<true /* TUseAppRep */, TAppRep, TSbx>,
    tainted_relocatable_pointer<true /* TUseAppRep */, TAppRep, TSbx>>;

}  // namespace rlbox
