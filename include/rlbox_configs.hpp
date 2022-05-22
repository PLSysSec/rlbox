/**
 * @file rlbox_configs.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This file exists purely to document the various macro configurations
 * that can be used to modify RLBox behavior
 * @details List of macro configurations supported by RLBox
 * - `RLBOX_CUSTOM_ABORT` - Specify that RLBox will call a custom abort handler
 * on errors. See @ref rlbox::detail::dynamic_check() for more details.
 * - `RLBOX_USE_EXCEPTIONS_ON_ERROR` - Specify that RLBox will throw an
 * exception on errors. See @ref rlbox::detail::dynamic_check() for more
 * details.
 * - `RLBOX_DISABLE_SANDBOX_CREATED_CHECKS` - Disable sandbox created checks on
 * APIs such as rlbox::rlbox_sandbox::create_sandbox(), @ref
 * rlbox::rlbox_sandbox::destroy_sandbox(), etc. See @ref
 * rlbox::rlbox_sandbox::create_status for more details.
 * - `RLBOX_REPLACE_COMPILE_CHECKS_WITH_RUNTIME_ERRORS` - Replace the static
 * checks used by rlbox to runtime errors (abort()/exceptions). This is meant to
 * be used primarily by the tests to ensure easy testing.
 */

#pragma once
