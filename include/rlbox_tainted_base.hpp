/**
 * @file rlbox_tainted_base.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Base classes for tainted types (wrappers that identify data returned
 * by sandboxed code).
 */

#pragma once

namespace rlbox {

/**
 * @brief Base class of all wrapper types which is used to identify tainted
 * wrappers.
 */
class tainted_interface {};

/**
 * @brief Base class of all wrapper types which is used to identify tainted
 * wrappers along with their sandbox type.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename TSbx>
class tainted_interface_sbx : public tainted_interface {};

/**
 * @brief Base class of all wrapper types with common template arguments. This
 * is used to identify tainted wrappers.
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename T, typename TSbx>
class tainted_any_base : public tainted_interface_sbx<TSbx> {
  /// \todo add UNSAFE_sandboxed and UNSAFE_unverified
};

}  // namespace rlbox
