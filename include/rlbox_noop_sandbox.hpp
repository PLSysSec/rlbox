#pragma once

/**
 * @file rlbox_noop_sandbox.hpp
 * @brief This header implements the rlbox_noop_sandbox class.
 */

#include "rlbox_types.hpp"

namespace rlbox {

/**
 * @brief A sandbox plugin implementation for a noop or "null" sandbox.
 * @details In this sandbox, memory accesses are not bounds checked and there is
 * no actual enforcement done. The only thing enforced by this sandbox is that
 * sandboxed data is tainted and the application must apply security checks to
 * this tainted data. The noop sandbox simply dispatches function calls as if
 * they were simple static function calls within the host application.
 */
class rlbox_noop_sandbox
{

public:
  /**
   * @brief Implementation of the RLBox create_sandbox API. For the
   * rlbox_noop_sandbox, this does nothing as all function calls or memory
   * allocations are forwarded to the application.
   *
   * @return rlbox_status_code indicates whether this function succeeded
   */
  rlbox_status_code impl_create_sandbox() { return rlbox_status_code::SUCCESS; }

  /**
   * @brief Implementation of the RLBox destroy_sandbox API. For the
   * rlbox_noop_sandbox, this does nothing.
   *
   * @return rlbox_status_code indicates whether this function succeeded
   */
  rlbox_status_code impl_destroy_sandbox()
  {
    return rlbox_status_code::SUCCESS;
  }
};

}