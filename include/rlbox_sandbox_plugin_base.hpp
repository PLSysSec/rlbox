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
   * from that of the host. In contrast to the `sbx_pointer` type name, which
   * indicates the size of the pointer, `sbx_pointer_is_different_rep` indicates
   * if the pointer representation is different even if the size is the same.
   * @details Note that if `sbx_pointer` is a different size from `void*`, the
   * `sbx_pointer_is_different_rep` field must be `true`
   */
  using sbx_pointer_is_different_rep = std::false_type;
};

}  // namespace rlbox

/**
 * @brief API used to invoke sandbox functions while also specifying the
 * function type. The parameters are expected to be rlbox wrapper types like
 * `tainted' types or `rlbox::rlbox_callback` types. This macro is used
 * internally by the sandbox plugin to specify how function invocations work for
 * this particular plugin. This macro allows
 *
 * The function type is usually just the decltype() of the function (which is
 * the case in the noop sandbox). However, this could be different in certain
 * cases described below.
 *
 * For plugins to sandboxes that change the ABI :  While these changes to the
 * ABI are generally automatically accounted for, plugins must take care to
 * handle standard sized types like uint32_t which are aliases to other types.
 * These types must be converted to rlbox standard int types such as
 * @ref rlbox::rlbox_uint32_t. Plugins such as the wasm2c sandbox plugin can
 * identify the use of standard int types directly by analyzing the resulting
 * ABI of compiled code. Plugins like NaCl sandbox may require the end user to
 * specify these.
 *
 * Usage
 * @code {.cpp} auto result = sandbox_invoke_internal(sandbox, int(int, int),
 * lib_add, 3, 4);
 * @endcode
 *
 * This can also be used with rlbox integer types to currently account for the
 * ABI changes
 * @code {.cpp} auto result = sandbox_invoke_internal(sandbox,
 * rlbox_uint32_t(rlbox_uint32_t, rlbox_uint32_t), lib_add, 3, 4);
 * @endcode
 * @details This macro ultimately forwards the call to a method on @ref
 * rlbox::rlbox_sandbox along with  a stringified version of the function name
 */
#define sandbox_invoke_internal(sandbox, func_type, func_name, func_ptr, ...) \
  sandbox.INTERNAL_invoke_sandbox_function<func_type>(#func_name, func_ptr,   \
                                                      ##__VA_ARGS__)
