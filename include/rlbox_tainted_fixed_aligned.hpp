/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_fixed_aligned type.
 */

#pragma once

#include <type_traits>

#include "rlbox_tainted_fixed_aligned_pointer.hpp"
#include "rlbox_tainted_fundamental_or_enum.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

template <bool TUseAppRep, typename TAppRep, typename TSbx>
using tainted_fixed_aligned_base = std::conditional_t<
    detail::is_fundamental_or_enum_v<detail::tainted_rep_t<TAppRep>>,
    tainted_fundamental_or_enum<TUseAppRep, TAppRep, TSbx>,
    tainted_fixed_aligned_pointer<TUseAppRep, TAppRep, TSbx>>;

template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_fixed_aligned_impl
    : public tainted_fixed_aligned_base<TUseAppRep, TAppRep, TSbx> {
 public:
#define RLBOX_FORWARD_TARGET_CLASS \
  tainted_fixed_aligned_base<TUseAppRep, TAppRep, TSbx>
#define RLBOX_FORWARD_CURR_CLASS tainted_fixed_aligned_impl
#define RLBOX_FORWARD_SUBCLASS

#include "rlbox_forwarder.hpp"
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
using tainted_fixed_aligned =
    tainted_fixed_aligned_impl<true /* TUseAppRep */, TAppRep, TSbx>;

}  // namespace rlbox
