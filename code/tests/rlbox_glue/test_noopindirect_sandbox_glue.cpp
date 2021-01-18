// NOLINTNEXTLINE
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
#define RLBOX_SINGLE_THREADED_INVOCATIONS
#include "rlbox_noopindirect_sandbox.hpp"

// NOLINTNEXTLINE
#define TestName "rlbox_noopindirect_sandbox"
// NOLINTNEXTLINE
#define TestType rlbox::rlbox_noopindirect_sandbox
// NOLINTNEXTLINE
#define CreateSandbox(sandbox) sandbox.create_sandbox()

#include "test_sandbox_glue.inc.cpp"