#pragma once

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#define RLBOX_USE_EXCEPTIONS
#include "rlbox.hpp"
#include "rlbox_noop_sandbox.hpp"
// IWYU pragma: end_exports

// IWYU pragma: no_include "rlbox_struct_support.hpp"

#include "libtest_structs_for_cpp_api.h"
rlbox_load_structs_from_library(libtest) // NOLINT
