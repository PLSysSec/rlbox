/**
 * @file rlbox_sandbox_plugin_base.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the rlbox_sandbox_plugin_base that acts as the
 * base class for rlbox_sandbox plugins
 */

#pragma once

#include "rlbox_tainted_relocatable.hpp"
#include "rlbox_tainted_volatile_standard.hpp"

namespace rlbox {

/**
 * @brief The base class for rlbox_sandbox plugins. This plugin contains
 * defaults for various plugin configurations such as which tainted type to use
 * and whether the sandboxed code uses the same ABI as the host
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename TSbx>
class rlbox_sandbox_plugin_base {
 public:
  /**
   * @brief The tainted type used by the underlying TSbx specification. This is
   * tainted_relocatable by default.
   *
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted = tainted_relocatable<T, TSbx>;

  /**
   * @brief The tainted_volatile type used by the underlying TSbx specification.
   * This is tainted_volatile_standard by default.
   *
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted_volatile = tainted_volatile_standard<T, TSbx>;
};

}  // namespace rlbox
