/**
 * @file rlbox_tainted_relocatable.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_relocatable type.
 */

#pragma once

#include <type_traits>

#include "rlbox_helpers.hpp"
#include "rlbox_tainted_fundamental_or_enum.hpp"
#include "rlbox_tainted_relocatable_pointer.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

template <typename TAppRep, typename TSbx>
using tainted_relocatable_base = std::conditional_t<
    detail::is_fundamental_or_enum_v<detail::tainted_rep_t<TAppRep>>,
    tainted_fundamental_or_enum<TAppRep, TSbx>,
    tainted_relocatable_pointer<TAppRep, TSbx>>;

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
class tainted_relocatable : public tainted_relocatable_base<TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 public:
#define RLBOX_FORWARD_TARGET_CLASS tainted_relocatable_base<TAppRep, TSbx>
#define RLBOX_FORWARD_CURR_CLASS tainted_relocatable
#define RLBOX_FORWARD_TO_SUBCLASS

#include "rlbox_forwarder.hpp"
};

}  // namespace rlbox
