#pragma once

/**
 * @file rlbox_tainted_relocatable.hpp
 * @brief This header implements the tainted_relocatable class.
 */

namespace rlbox {
/**
 * @brief Implementation of tainted data wrappers that supports the movement of
 * the sandbox heap after creation.
 *
 * @details If the tainted data is a pointer type, the value is stored as a
 * relative offset from the heap base. The offset is added to the heap base
 * when the pointer is accessed.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template<typename T, typename TSbx>
class tainted_relocatable
{};
}
