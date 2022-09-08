/**
 * @file rlbox_sandbox_plugin_base.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the rlbox_sandbox_plugin_base that acts as the
 * base class for rlbox_sandbox plugins
 */

#pragma once

#include <stddef.h>
#include <type_traits>

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

  /**
   * @brief How can the sandbox type `wchar_t` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_wchart = wchar_t;
  /**
   * @brief How can the sandbox type `short` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_short = short;
  /**
   * @brief How can the sandbox type `int` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_int = int;
  /**
   * @brief How can the sandbox type `long` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_long = long;
  /**
   * @brief How can the sandbox type `longlong` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_longlong = long long;
  /**
   * @brief How can the sandbox type `size_t` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_sizet = size_t;
  /**
   * @brief How can the sandbox type `pointer` be represented in the host
   * environment. Defaults to same as the host type. This can be overridden by
   * the sandbox plugin if the sandbox uses a custom ABI.
   */
  using sbx_pointer = void*;
  /**
   * @brief Indicates if the sandbox type `pointer` is represented differently
   * from that of the host. In contrast to the `sbx_pointer` alias, which
   * indicates the size of the pointer, `sbx_pointer_is_different_rep` indicates
   * if the pointer representation is different even if the size is the same.
   * @details Note that if `sbx_pointer` is of different size, than `void*` the
   * qsbx_pointer_is_different_rep` must be `true`
   */
  using sbx_pointer_is_different_rep = std::false_type;
};

}  // namespace rlbox
