/**
 * @file rlbox_sandbox.hpp
 * @copyright This project is released under the MIT License.
 * You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the rlbox_sandbox class.
 */

#pragma once

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
#  include <atomic>
#endif
#include <utility>

#include "rlbox_error_handling.hpp"
#include "rlbox_types.hpp"

namespace rlbox {

/**
 * @brief An rlbox sandbox type that provides APIs to invoke functions in the
 * sandboxed component, expose callbacks, retrieve information about the sandbox
 * memory etc.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation. For the "null" sandbox, the value is
 * rlbox_noop_sandbox.
 */
template <typename TSbx>
class rlbox_sandbox : protected TSbx {
 private:
  /**
   * @brief This type tracks the state of sandbox creation and is for internal
   * use only. This state is checked prior to any operations on the sandbox from
   * the host program.
   * @details We should ideally check this state to see if the sandbox is in the
   * created state during sandbox operations such as invoking functions.
   * However, it is expensive to check in APIs such as sandbox_invoke or in the
   * callback_interceptor. In general, we leave it up to the user to ensure APIs
   * such as sandbox_invoke are never called prior to sandbox construction or
   * after destruction. We only perform checks suring create_sandbox,
   * detroy_sandbox and register_callback where they will not add too much
   * overhead. Even this limited checking can be diabled through the macro
   * RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
   */
  enum class create_status { NOT_CREATED, INITIALIZING, DESTRUCTING, CREATED };

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
  /**
   * @brief This variable tracks the creation status of the sandbox instance
   */
  std::atomic<create_status> sandbox_created = create_status::NOT_CREATED;
#endif

 public:
  /**
   * @brief The tainted type used by the underlying TSbx specification.
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted = typename TSbx::template tainted<T>;

  /**
   * @brief The tainted_volatile type used by the underlying TSbx specification.
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted_volatile = typename TSbx::template tainted_volatile<T>;

  /**
   * @brief API invoked to initialize a new sandbox. This function should be run
   * to completion before any functions are invoked in this sandbox.
   *
   * @tparam TArgs is the type of the sandbox creation parameters. The value
   * here is specific to the plugin implementation.
   * @param aArgs specifies any sandbox creation parameters. The value here is
   * specific to the plugin implementation.
   * @return rlbox_status_code indicates whether this function succeeded
   */
  template <typename... TArgs>
  rlbox_status_code create_sandbox(TArgs... aArgs) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    auto expected = create_status::NOT_CREATED;
    bool success = sandbox_created.compare_exchange_strong(
        expected, create_status::INITIALIZING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already created/is being "
        "created concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    rlbox_status_code ret =
        this->impl_create_sandbox(std::forward<TArgs>(aArgs)...);

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    if (ret == rlbox_status_code::SUCCESS) {
      sandbox_created.store(create_status::CREATED);
    } else {
      sandbox_created.store(create_status::NOT_CREATED);
    }
#endif

    return ret;
  }

  /**
   * @brief Destroy the sandbox. This function should be called only after a
   * successful call to create_sandbox. After destruction, the application
   * should assume the sandbox no longer has memory and can no longer execute
   * function calls.
   *
   * @return rlbox_status_code indicates whether this function succeeded
   */
  rlbox_status_code destroy_sandbox() {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    auto expected = create_status::CREATED;
    bool success = sandbox_created.compare_exchange_strong(
        expected, create_status::DESTRUCTING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already destroyed/is being "
        "destroyed concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    rlbox_status_code ret = this->impl_destroy_sandbox();
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    sandbox_created.store(create_status::NOT_CREATED);
#endif
    return ret;
  }
};

/**
 * @brief API used to invoke sandbox functions while also specifying the
 * function type. The parameters are expected to be rlbox wrapper types like
 * `tainted' types or `rlbox::rlbox_callback` types.
 *
 * This macro is used internally in most cases.
 *
 * However, end users must invoke sandbox functions with this macro IFF the
 * sandboxing technology both
 *   - Changes the ABI of types like int, long etc.
 *   - Does not expose the target type of compiled code
 *
 * This means
 *   - Users of the rlbox_noop_sandbox (does not change ABI) will NOT need this
 *   - Users of the rlbox_wasm2c_sandbox (changes the ABI, but exposes the
 * target type of the function) will NOT need this
 *   - Users of the rlbox_nacl_sandbox (changes the ABI, does not expose the
 * target type of the function) will need this
 *
 * For e.g.,
 * @code {.cpp}
 * auto result = sandbox_invoke_with_func_type(sandbox, int(int, int), lib_add,
 * 3, 4);
 * @endcode
 *
 * This can also be used with rlbox integer types to currently account for the
 * ABI changes
 * @code {.cpp}
 * auto result = sandbox_invoke_with_func_type(sandbox,
 * rlbox_uint32_t(rlbox_uint32_t, rlbox_uint32_t), lib_add, 3, 4);
 * @endcode
 * @details This macro ultimately forwards the call to a method on @ref
 * rlbox::rlbox_sandbox along with  a stringified version of the
 * function name
 */
#define sandbox_invoke_with_func_type(sandbox, func_type, func_name, ...) \
  sandbox.invoke_sandbox_function<func_type>(                             \
      #func_name, rlbox_wasm2c_sandbox_symbol_res(func_name), _VA_ARGS_...)

/**
 * @brief API used to invoke sandbox functions. The parameters are expected to
 * be rlbox wrapper types like `tainted' types or `rlbox::rlbox_callback` types.
 *
 * For e.g.,
 * @code {.cpp}
 * auto result = sandbox_invoke(sandbox, lib_add, 3, 4);
 * @endcode
 * @details This macro ultimately forwards the call to a method on @ref
 * rlbox::rlbox_sandbox along with the function type.
 */
#define sandbox_invoke(sandbox, func_name, ...)                          \
  sandbox_invoke_with_func_type(sandbox, decltype(func_name), func_name, \
                                _VA_ARGS_...)
}  // namespace rlbox