/**
 * @file test_plugin_include.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Include file to be used by plugin tests. Contains some common headers
 * and functions used by plugin tests
 */

#pragma once

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

#include "rlbox.hpp"
// IWYU pragma: end_exports

using namespace rlbox;

// RLBox's plugin tests should be customized to run on a specific sandbox
// plugin. This can be done by specifying TestName, TestType and CreateSandbox
// macros. To ensure intellisense works, we default this to rlbox_noop_sandbox

/**
 * @brief The TestName macro should specify a unique name for the test
 */
#ifndef TestName
#  error "Define TestName before including this file"
#  define TestName "rlbox_noop_sandbox"
#endif

/**
 * @brief The TestType macro should specify which sandbox plugin
 * is being tested
 */
#ifndef TestType
#  error "Define TestType before including this file"
#  define TestType rlbox_noop_sandbox
#endif

/**
 * @brief The CreateSandbox macro should specify how to initialize the sandbox
 * plugin
 */
#ifndef CreateSandbox
#  error "Define CreateSandbox before including this file"
#  define CreateSandbox(s) s.create_sandbox()
#endif

#include "rlbox_noop_sandbox.hpp"
