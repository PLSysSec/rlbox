#pragma once

/**
 * @file rlbox_tainted_base.hpp
 * @brief Base classes for tainted types (wrappers that identify data returned
 * by sandboxed code). These classes contain common functions used by different
 * tainted and tainted_volatile implementations.
 */

// IWYU incorrectly reports this as unnecessary as the use of type_traits is in
// a templated class
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_error_handling.hpp"

namespace rlbox {

/**
 * @brief Base classes for tainted types when the template type T is a primitive
 * type (int, int[], float, pointers etc.)
 *
 * @details Class types get specializations for the tainted classes, for example
 * tainted_volatile<Foo> and tainted_volatile<Bar>. If this class is initialized
 * with a T that is a class type, this means we are missing the tainted
 * specialization for the class type. This class checks for this case and
 * provides helpful error messages.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_primitive_base {
  // Check that this is used only for primitive types
  // If initialized for a class type, give a helpful error message
  static_assert(!std::is_class_v<T>,
                "Missing rlbox definition for class T.\n"
                "To support use of tainted<T> where T is a class/struct type, "
                "RLBox requires:\n"
                "1) Classes to be described in the rlbox_lib_class_T macro\n"
                "2) Programs to call the rlbox_lib_load_classes "
                "macro\n" RLBOX_REFER_DOCS_MESSAGE "\n");
};

}  // namespace rlbox
