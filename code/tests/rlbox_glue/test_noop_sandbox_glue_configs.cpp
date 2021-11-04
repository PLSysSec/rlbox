// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
#define RLBOX_SINGLE_THREADED_INVOCATIONS
#define RLBOX_EMBEDDER_PROVIDES_TLS_STATIC_VARIABLES
#include "rlbox_noop_sandbox.hpp"

// NOLINTNEXTLINE
#define TestName "rlbox_noop_sandbox configs"
// NOLINTNEXTLINE
#define TestType rlbox::rlbox_noop_sandbox
// NOLINTNEXTLINE
#define CreateSandbox(sandbox) sandbox.create_sandbox()

RLBOX_NOOP_SANDBOX_STATIC_VARIABLES();

#include "test_sandbox_glue.inc.cpp"