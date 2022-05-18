/**
 * @file rlbox.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header is the top level include of rlbox that exports all
 * functionality included in the rlbox library such as sandbox creation,
 * invoking functions in sandboxes, tainting and untainting of data etc.
 * @note This header does not export the rlbox_noop_sandbox. That header must be
 * pulled in explicitly if required.
 */

#pragma once

/** @mainpage RLBox documentation
 *
 * RLBox is a toolkit to securely sandbox third-party libraries
 * ([source](https://rlbox.dev), [tutorial](https://docs.rlbox.dev)).
 *
 * This document contains the inline code documentation of RLBox.
 *
 * The @ref rlbox::rlbox_sandbox class snf the @ref rlbox_types.hpp file are
 * useful starting points for this document.
 */

// IWYU pragma: begin_exports

#include "rlbox_sandbox.hpp"
#include "rlbox_types.hpp"

// Note we do not export rlbox_noop_sandbox here.
// Developers must explicitly include this if they want this.

// IWYU pragma: end_exports
