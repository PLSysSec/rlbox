#pragma once

/**
 * @file rlbox.hpp
 * @brief This header is the top level include of rlbox that exports all
 * functionality included in the rlbox library such as sandbox creation,
 * invoking functions in sandboxes, tainting and untainting of data etc.
 * @note This header does not export the rlbox_noop_sandbox. That header must be
 * pulled in explicitly if required.
 */

// IWYU pragma: begin_exports

#include "rlbox_sandbox.hpp"
#include "rlbox_types.hpp"

// Note we do not export rlbox_noop_sandbox here.
// Developers must explicitly include this if they want this.

// IWYU pragma: end_exports
