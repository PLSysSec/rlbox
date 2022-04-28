/**
 * @file test_all.cpp
 * @brief Meta file that includes all other plugin tests in one large cpp file.
 * @details This makes it easy for any sandbox plugin to simply include this one
 * file with appropriate macro definitions of TestName, TestType, CreateSandbox
 * and test features. See file test_rlbox_noop_sandbox.cpp for an example.
 */

#include "test_create_destroy.cpp"