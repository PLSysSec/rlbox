// NOLINTNEXTLINE
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
#define RLBOX_SINGLE_THREADED_INVOCATIONS
#include "rlbox_dylib_sandbox.hpp"

// NOLINTNEXTLINE
#define TestName "rlbox_dylib_sandbox"
// NOLINTNEXTLINE
#define TestType rlbox::rlbox_dylib_sandbox

#ifndef GLUE_LIB_PATH
#  error "Missing definition for GLUE_LIB_PATH"
#endif

#if defined(_WIN32)
// NOLINTNEXTLINE
#  define CreateSandbox(sandbox) sandbox.create_sandbox(L"" GLUE_LIB_PATH)
#else
// NOLINTNEXTLINE
#  define CreateSandbox(sandbox) sandbox.create_sandbox(GLUE_LIB_PATH)
#endif

#include "test_sandbox_glue.inc.cpp"
