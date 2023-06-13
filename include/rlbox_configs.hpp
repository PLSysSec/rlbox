/**
 * @file rlbox_configs.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This file exists purely to document the various macro configurations
 * that can be used to modify RLBox behavior
 * @details List of macro configurations supported by RLBox
 *
 * - `RLBOX_CUSTOM_ABORT` - Define this to specify that RLBox will call a custom
 *   abort handler on errors. See @ref rlbox::detail::dynamic_check() for more
 *   details.
 *
 * - `RLBOX_USE_EXCEPTIONS_ON_ERROR` - Define this to specify that RLBox will
 *   throw an exception on errors. See @ref rlbox::detail::dynamic_check() for
 *   more details.
 *
 * - `RLBOX_DISABLE_SANDBOX_CREATED_CHECKS` - Define this to disable sandbox
 *   created checks on APIs such as rlbox::rlbox_sandbox::create_sandbox(), @ref
 *   rlbox::rlbox_sandbox::destroy_sandbox(), etc. See @ref
 *   rlbox::rlbox_sandbox::create_status for more details.
 *
 * - `RLBOX_REPLACE_COMPILE_CHECKS_WITH_RUNTIME_ERRORS` - Define this to replace
 *   the static checks used by rlbox to runtime errors (abort()/exceptions).
 *   This is meant to be used primarily by the tests to ensure easy testing.
 *
 * - `RLBOX_DONT_OVERESTIMATE_CLASS_SIZES` - Define to disable the RLBox
 *   approximations that allow calling @ref
 *   rlbox::rlbox_sandbox::malloc_in_sandbox() to allocate space for a class
 *   without defining the class layout in `rlbox_lib_load_classes`. When this
 *   config is not defined, RLBox checks to see if the class size in the host
 *   ABI will always be greater than the class size in the sandbox ABI. If this
 *   is the case, then RLBox just uses the host ABI class size. When this config
 *   is defined, RLBox requires defining the class layout in
 * `rlbox_lib_load_classes`. See @ref
 * rlbox::rlbox_sandbox::get_object_size_for_malloc for more details.
 */

#pragma once
