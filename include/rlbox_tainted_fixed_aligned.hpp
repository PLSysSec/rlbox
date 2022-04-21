#pragma once

/**
 * @file rlbox_tainted_fixed_aligned.hpp
 * @brief This header implements the tainted_fixed_aligned class.
 */

namespace rlbox {
/**
 * @brief Implementation of tainted data wrappers that requires a fixed sandbox
 * heap aligned to its size.
 *
 * @details Tainted data wrappers (wrappers that identify data returned by
 * sandboxed code) that requires:
 * - the sandbox heap cannot be moved once created (fixed). The heap maybe grown
 * in-place.
 * - the sandbox heap is aligned to its size (aligned), even on growth.
 *      E.g., a heap of max size 4gb is aligned to 4gb
 *
 * Due to these assumptions, if the tainted data is a pointer type, the value is
 * stored as a global pointer.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template<typename T, typename TSbx>
class tainted_fixed_aligned
{};
}
