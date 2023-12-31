/**
 * @file rlbox_types.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header forward declares rlbox types like rlbox_sandbox, and also
 * provides the RLBOX_DEFINE_BASE_TYPES_FOR macro which allows declaring
 * tainted_foo for any library "foo"
 */

#pragma once

#include "rlbox_helpers.hpp"

// IWYU pragma: begin_exports

#include "rlbox_stdint_types.hpp"

// IWYU pragma: end_exports

namespace rlbox {

template <typename TSbx>
class rlbox_sandbox;  // IWYU pragma: keep

/**
 * @brief A wrapper type used to mark any function pointers to application code
 * that is passed to the sandbox. This type indicates that the application has
 * explicitly allowed this function to be called by the sandbox.
 *
 * @tparam T is the type of the data that is wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class rlbox_callback;  // IWYU pragma: keep

/**
 * @brief A wrapper type used to mark any function pointers to sandbox code that
 * is passed to the sandbox.
 *
 * @tparam T is the type of the data that is wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class rlbox_sandboxed_function;  // IWYU pragma: keep

/**
 * @brief A wrapper type used to mark any raw pointers to application data
 * structures that is passed to the sandbox.
 * @details This type passes in a unique opaque integer to the sandbox to
 * ensure there is no pointer leaked to sandboxed code When the sandbox returns
 * this integer, it can be looked up in a map to recover the original data
 * structure pointer.
 * @tparam T is the type of the data that is wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class rlbox_app_pointer;  // IWYU pragma: keep

/**
 * @brief Tainted boolean that serves as a "hint" and not a definite answer
 * @details Comparisons with memory controlled by the sandbox (tainted_volatile
 * data) return such hints. They are not `tainted<bool>` values because a
 * compromised sandbox can modify tainted_volatile data at any time meaning the
 * hint could potentially be incorrect.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_boolean_hint;  // IWYU pragma: keep

/**
 * @brief Tainted int that serves as a "hint" and not a definite answer
 * @details Spaceship style comparisons that return (less, equal, greater) in
 * the form of an integer return this. For example strcmp. Comparisons with
 * memory controlled by the sandbox (tainted_volatile data) return such hints.
 * They are not `tainted<bool>` values because a compromised sandbox can modify
 * tainted_volatile data at any time meaning the hint could potentially be
 * incorrect.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_int_hint;  // IWYU pragma: keep

/**
 * @brief A sandbox plugin implementation for a noop or "null" sandbox.
 * @details In this sandbox, memory accesses are not bounds checked and there is
 * no actual enforcement done. The only thing enforced by this sandbox is that
 * sandboxed data is tainted and the application must apply security checks to
 * this tainted data. The noop sandbox simply dispatches function calls as if
 * they were simple static function calls within the host application.
 */
class rlbox_noop_sandbox;  // IWYU pragma: keep

/**
 * @brief A memory management class the implements unique_ptr for tainted types.
 * @details Tainted types are not compatible with std::unique_ptr (as
 * std::unique_ptr requires the managed type to be a pointer while RLBox tainted
 * types manage pointers are structs). This class thus provides the an
 * unique_ptr compatible with tainted types. The class interface tries to follow
 * the std::unique_ptr as much as possible.
 * @tparam T is the type of the pointer being managed. For example, to manage a
 * `tainted<int*>`, developers would use a `rlbox_unique_ptr<int>`
 * @tparam TSbx is the type of the sandbox plugin that represents the
 * underlying sandbox implementation.
 */
template <typename T, typename TSbx>
class rlbox_unique_ptr;  // IWYU pragma: keep

/**
 * @brief A type representing the success/error code for various rlbox related
 * operations
 */
enum class rlbox_status_code {
  SUCCESS,
  SANDBOX_CREATE_FAILURE,
  SANDBOX_DESTROY_FAILURE
};

/**
 * @brief This macro specialized types @ref rlbox::rlbox_sandbox, the tainted
 * types, etc. for a particular sandbox type.
 * @details For example
 * @code {.cpp}
 * RLBOX_DEFINE_BASE_TYPES_FOR(libtest, rlbox_noop_sandbox);
 * @endcode
 *
 * The above code creates the types `rlbox_sandbox_libtest` (which is an alias
 * to `rlbox_sandbox<rlbox_noop_sandbox>`), rlbox_callback_libtest<T> (which is
 * an alias to `rlbox_callback<T, rlbox_noop_sandbox>`)
 *
 * This call will be followed by a define for the appropriate sandbox_invoke
 * @code {.cpp}
 * #define libtest_sandbox_invoke noop_sandbox_invoke
 * @endcode
 */
#define RLBOX_DEFINE_BASE_TYPES_FOR(SBXNAME, SBXTYPE)                        \
  namespace rlbox {                                                          \
  class SBXTYPE;                                                             \
  }                                                                          \
                                                                             \
  using rlbox_sandbox_type_##SBXNAME = rlbox::SBXTYPE;                       \
                                                                             \
  using rlbox_sandbox_##SBXNAME =                                            \
      rlbox::rlbox_sandbox<rlbox_sandbox_type_##SBXNAME>;                    \
                                                                             \
  template <typename T>                                                      \
  using tainted_##SBXNAME = rlbox_sandbox_##SBXNAME::tainted<T>;             \
                                                                             \
  template <typename T>                                                      \
  using tainted_volatile_##SBXNAME =                                         \
      rlbox_sandbox_##SBXNAME::tainted_volatile<T>;                          \
                                                                             \
  template <typename T>                                                      \
  using rlbox_unique_ptr_##SBXNAME =                                         \
      rlbox::rlbox_unique_ptr<T, rlbox_sandbox_type_##SBXNAME>;              \
                                                                             \
  using tainted_boolean_hint_##SBXNAME =                                     \
      rlbox::tainted_boolean_hint<true, bool, rlbox_sandbox_type_##SBXNAME>; \
                                                                             \
  RLBOX_REQUIRE_SEMI_COLON

// In the future the following will also be added
// template<typename T>
// using rlbox_callback_##SBXNAME =
//   rlbox::rlbox_callback<T, rlbox_sandbox_type_##SBXNAME>;

// template<typename T>
// using rlbox_sandboxed_function_##SBXNAME =
//   rlbox::rlbox_sandboxed_function<T, rlbox_sandbox_type_##SBXNAME>;

// template<typename T>
// using rlbox_app_pointer_##SBXNAME =
//   rlbox::rlbox_app_pointer<T, rlbox_sandbox_type_##SBXNAME>;

// using tainted_integer_hint_##SBXNAME = rlbox::tainted_integer_hint;

}  // namespace rlbox