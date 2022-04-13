#pragma once

namespace rlbox {

// The type that represents a sandbox. This type provides APIs to invoke
// functions in the sandboxed component, expose callbacks, retrieve informtion
// about the sandbox memory etc. TSbx is the type of the sandbox plugin that
// represents the underlying sandbox implementation.
template<typename TSbx>
class rlbox_sandbox; // IWYU pragma: keep

// A wrapper type used to mark any function pointers to application code that is
// passed to the sandbox. T is the type of the data, TSbx is type of the sandbox
// plugin. This type indicates that the application has explicitly allowed this
// function to be called by the sandbox.
template<typename T, typename TSbx>
class rlbox_callback; // IWYU pragma: keep

// A wrapper type used to mark any function pointers to sandbox code that is
// passed to the sandbox. T is the type of the data, TSbx is type of the sandbox
// plugin.
template<typename T, typename TSbx>
class rlbox_sandboxed_function; // IWYU pragma: keep

// A wrapper type used to mark any raw pointers to application data structures
// that is passed to the sandbox. T is the type of the data, TSbx is type of the
// sandbox plugin. This type passes in a unique opaque integer to the sandbox to
// ensure there is no pointer leaked to sandboxed code When the sandbox returns
// this integer, it can be looked up in a map to recover the original data
// structure pointer.
template<typename T, typename TSbx>
class rlbox_app_pointer; // IWYU pragma: keep

// Tainted values that serves as a "hint" and not a definite answer
// Comparisons with memory controlled by the sandbox (tainted_volatile data)
// return such hints. They are not `tainted<bool>` values because a compromised
// sandbox can modify tainted_volatile data at any time meaning the hint could
// potentially be incorrect.
class tainted_boolean_hint; // IWYU pragma: keep
class tainted_int_hint;     // IWYU pragma: keep

// A "sandbox plugin" implementation for a noop sandbox. In this sandbox, memory
// accesses are not bounds checked and there is no actual enforcement done. The
// only thing enforced by this sandbox is that sandboxed data is tainted and the
// application must apply security checks to this tainted data. The noop sandbox
// simply dispatches function calls as if they were simple static function calls
// within the host application.
class rlbox_noop_sandbox; // IWYU pragma: keep

// A type representing the success/error code for various rlbox related
// operations
enum class rlbox_status_code
{
  SUCCESS,
  SANDBOX_CREATE_FAILURE,
  SANDBOX_DESTROY_FAILURE
};

#define RLBOX_DEFINE_BASE_TYPES_FOR(SBXNAME, SBXTYPE)                          \
  namespace rlbox {                                                            \
    class SBXTYPE;                                                             \
  }                                                                            \
                                                                               \
  using rlbox_sandbox_type_##SBXNAME = rlbox::SBXTYPE;                         \
                                                                               \
  using rlbox_sandbox_##SBXNAME =                                              \
    rlbox::rlbox_sandbox<rlbox_sandbox_type_##SBXNAME>;                        \
                                                                               \
  template<typename T>                                                         \
  using tainted_##SBXNAME = rlbox_sandbox_##SBXNAME::tainted<T>;

// In the future the following will also be added
// template<typename T>
// using tainted_volatile_##SBXNAME =
//   rlbox_sandbox_##SBXNAME::tainted_volatile;

// template<typename T>
// using rlbox_callback_##SBXNAME =
//   rlbox::rlbox_callback<T, rlbox_sandbox_type_##SBXNAME>;

// template<typename T>
// using rlbox_sandboxed_function_##SBXNAME =
//   rlbox::rlbox_sandboxed_function<T, rlbox_sandbox_type_##SBXNAME>;

// template<typename T>
// using rlbox_app_pointer_##SBXNAME =
//   rlbox::rlbox_app_pointer<T, rlbox_sandbox_type_##SBXNAME>;

// using tainted_boolean_hint_##SBXNAME = rlbox::tainted_boolean_hint;
// using tainted_integer_hint_##SBXNAME = rlbox::tainted_integer_hint;

};