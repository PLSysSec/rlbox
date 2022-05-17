/**
 * @file rlbox_configs.hpp
 * @brief This file exists purely to document the various macro configurations
 * that can be used to modify RLBox behavior
 * @details List of macro configurations supported by RLBox
 * - RLBOX_CUSTOM_ABORT - Specify that RLBox will call a custom abort handler on
 * errors. See \ref rlbox::detail::dynamic_check() for more details.
 * - RLBOX_USE_EXCEPTIONS_ON_ERROR - Specify that RLBox will throw an exception
 * on errors. See \ref rlbox::detail::dynamic_check() for more details.
 * - RLBOX_DISABLE_SANDBOX_CREATED_CHECKS - Disable sandbox created checks on
 * \ref APIs such as rlbox::rlbox_sandbox::create_sandbox(), \ref
 * rlbox::rlbox_sandbox::destroy_sandbox(), etc. See \ref
 * rlbox::rlbox_sandbox::create_status for more details.
 */

#pragma once
