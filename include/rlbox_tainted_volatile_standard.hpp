#pragma once

/**
 * @file rlbox_tainted_volatile_standard.hpp
 * @brief This header implements the tainted_volatile_standard class.
 */

namespace rlbox {
/**
 * @brief Implementation of tainted_volatile data wrapper. tainted_volatile_standard
 * indicates that this data is memory that can be modified by the sandbox.
 *
 * @details tainted_volatile_standard data in addition to being untrusted (since it is
 * memory written to be sandboxed code), should be carefully checked for
 * double-read or time-of-check-time-of-use style bugs, i.e., if this data is
 * read twice, the data could have been changed asynchronously by a concurrent
 * sandbox thread. To safely handle tainted_volatile_standard data, the host application
 * should make a copy of tainted_volatile_standard data into the application memory
 * (tainted data) or call copy_and_verify to make a sanitized copy of this data.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template<typename T, typename TSbx>
class tainted_volatile_standard
{};
}
