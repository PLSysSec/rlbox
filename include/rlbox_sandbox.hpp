#pragma once

/**
 * @file rlbox_sandbox.hpp
 * @brief This header implements the rlbox_sandbox class.
 */

#include <utility>

#include "rlbox_tainted_relocatable.hpp"
#include "rlbox_tainted_volatile_standard.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief An rlbox sandbox type tha provides APIs to invoke functions in the
 * sandboxed component, expose callbacks, retrieve information about the sandbox
 * memory etc.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation. For the "null" sandbox, the value is
 * rlbox_noop_sandbox.
 */
template<typename TSbx>
class rlbox_sandbox : protected TSbx
{

private:
  /**
   * @brief The default implementation of tainted unless overridden by TSbx
   */
  template<typename T>
  using TDefaultTainted = tainted_relocatable<T, TSbx>;

  /**
   * @brief The default implementation of tainted_volatile unless overridden by
   * TSbx
   */
  template<typename T>
  using TDefaultTaintedVolatile = tainted_volatile_standard<T, TSbx>;

public:
  /**
   * @brief The tainted type used by the underlying TSbx specification. If
   * unspecified this is tainted_relocatable by default.
   *
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template<typename T>
  using tainted =
    detail::get_typemember_tainted_or_default_t<TSbx, TDefaultTainted, T>;

  /**
   * @brief The tainted_volatile type used by the underlying TSbx specification.
   * If unspecified this is tainted_volatile_standard by default.
   *
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template<typename T>
  using tainted_volatile = detail::get_typemember_tainted_volatile_or_default_t<
    TSbx,
    TDefaultTaintedVolatile,
    T>;

  /**
   * @brief API invoked to initialize a new sandbox. This function should be run
   * to completion before any functions are invoked in this sandbox.
   *
   * @tparam TArgs is the type of the sandbox creation parameters. The value
   * here is specific to the plugin implementation.
   * @param params specifies any sandbox creation parameters. The value here is
   * specific to the plugin implementation.
   * @return rlbox_status_code indicates whether this function succeeded
   */
  template<typename... TArgs>
  rlbox_status_code create_sandbox(TArgs... args)
  {
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    return this->impl_create_sandbox(std::forward<TArgs>(args)...);
  }

  /**
   * @brief Destroy the sandbox. This function should be called only after a
   * successful call to create_sandbox. After destruction, the application
   * should assume the sandbox no longer has memory and can no longer execute
   * function calls.
   *
   * @return rlbox_status_code indicates whether this function succeeded
   */
  rlbox_status_code destroy_sandbox()
  {
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    return this->impl_destroy_sandbox();
  }
};

}