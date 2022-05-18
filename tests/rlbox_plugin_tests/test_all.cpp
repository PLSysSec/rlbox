/**
 * @file test_all.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Meta file that includes all other plugin tests in one large cpp file.
 * @details This makes it easy for any sandbox plugin to simply include this one
 * file with appropriate macro definitions of TestName, TestType, CreateSandbox
 * and test features. See file test_rlbox_noop_sandbox.cpp for an example.
 */

#include "test_create_destroy.cpp"