/**
 * @file rlbox_tainted_base.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Base classes for tainted types (wrappers that identify data returned
 * by sandboxed code). These classes contain common functions used by different
 * tainted and tainted_volatile implementations.
 */

#pragma once

namespace rlbox {

/**
 * @brief Base class of all wrapper types. This is used to identify tainted
 * wrappers.
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_any_base {};

/**
 * @brief Base class of tainted types.
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_base : public tainted_any_base<T, TSbx> {};

/**
 * @brief Base class of tainted volatile types.
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_volatile_base : public tainted_any_base<T, TSbx> {};

}  // namespace rlbox
