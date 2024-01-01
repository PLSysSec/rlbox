/**
 * @file rlbox_tainted_fundamental_or_enum.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header specializes tainted_fundamental_or_enum_base class to
 * classes that handle tainted and tainted_volatile specifically.
 */

#pragma once

#include "rlbox_helpers.hpp"
#include "rlbox_tainted_fundamental_or_enum_base.hpp"

namespace rlbox {

template <typename TAppRep, typename TSbx>
class tainted_fundamental_or_enum
    : public tainted_fundamental_or_enum_base<true /* TUseAppRep */, TAppRep,
                                              TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 public:
#define RLBOX_FORWARD_TARGET_CLASS \
  tainted_fundamental_or_enum_base<true /* TUseAppRep */, TAppRep, TSbx>
#define RLBOX_FORWARD_CURR_CLASS tainted_fundamental_or_enum
#define RLBOX_FORWARD_TO_SUBCLASS

#include "rlbox_forwarder.hpp"
};

template <typename TAppRep, typename TSbx>
class tainted_volatile_fundamental_or_enum
    : public tainted_fundamental_or_enum_base<false /* TUseAppRep */, TAppRep,
                                              TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 public:
#define RLBOX_FORWARD_TARGET_CLASS \
  tainted_fundamental_or_enum_base<false /* TUseAppRep */, TAppRep, TSbx>
#define RLBOX_FORWARD_CURR_CLASS tainted_volatile_fundamental_or_enum
#define RLBOX_FORWARD_TO_SUBCLASS

#include "rlbox_forwarder.hpp"
};

}  // namespace rlbox
