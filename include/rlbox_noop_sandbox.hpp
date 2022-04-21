#pragma once

/**
 * @file rlbox_noop_sandbox.hpp
 * @brief This header implements the rlbox_noop_sandbox class.
 */

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
{};
}