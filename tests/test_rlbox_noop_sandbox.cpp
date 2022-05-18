/**
 * @file test_rlbox_noop_sandbox.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Testing the rlbox_noop_sandbox plugin with the tests in the
 * rlbox_plugin_tests folder
 */

// Specify the configurations for test. See test_plugin_include.hpp for more
// details.
#define TestName "rlbox_noop_sandbox"
#define TestType rlbox_noop_sandbox
#define CreateSandbox(sandbox) sandbox.create_sandbox()

// Include the header that has the plugin to be tested
#include "rlbox_noop_sandbox.hpp"

// Include the tests from rlbox_plugin_tests folder
#include "rlbox_plugin_tests/test_all.cpp"
