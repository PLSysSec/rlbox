#pragma once

#include <type_traits>

#include "rlbox_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

template<typename T, typename T_Sandbox>
class tainted;

template<typename T, typename T_Sandbox>
class tainted_volatile;

/* Trait types */

class sandbox_wrapper_base
{};

template<typename T>
class sandbox_wrapper_base_of
{};

template<typename T, typename T_Sandbox>
class tainted_base
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T>
{};

// Check to ensure we don't accidentally introduce vtables which would slow
// things down
static_assert(sizeof(sandbox_wrapper_base) < sizeof(uintptr_t));
static_assert(sizeof(sandbox_wrapper_base_of<int>) < sizeof(uintptr_t));
static_assert(sizeof(tainted_base<int, int>) < sizeof(uintptr_t));

////////////////

template<typename T, typename T_Sandbox>
class tainted : public tainted_base<T, T_Sandbox>
{
  KEEP_CLASSES_FRIENDLY

  // Classes recieve their own specialization
  static_assert(!std::is_class_v<T>,
                "Missing specialization for class T. This error occurs for one "
                "of 2 reasons.\n"
                "  1) Make sure you have include a call rlbox_load_library_api "
                "for this library.\n"
                "  2) Make sure you run (re-run) the struct-dump tool to list "
                "all structs in use by your program.\n");

private:
  using T_ConvertedType = typename T_Sandbox::template convert_sandbox_t<T>;
  T data;

  inline T get_raw_value() const noexcept { return data; }

  inline T_ConvertedType get_raw_sandbox_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted<ptrs> can only be null or a pointer referring to a
      // location in sandbox memory, data can thus be the
      // example_unsandboxed_ptr
      return T_Sandbox::get_sandboxed_pointer(
        data, data /* example_unsandboxed_ptr */);
    } else {
      return adjust_type_size<T_ConvertedType>(data);
    }
  };

  inline void assign_value(const tainted_volatile<T, T_Sandbox>& val)
  {
    using namespace std;
    using T_ElementType = remove_all_extents_t<T>;

    if_constexpr_named(cond1, is_fundamental_or_ptr_like_v<T>)
    {
      data = val.get_raw_value();
    }
    else if_constexpr_named(cond2,
                            is_array_v<T> && is_fundamental_v<T_ElementType> &&
                              sizeof(T) == sizeof(T_ConvertedType))
    {
      // Something like an int[5] and the machine models are identical
      // we can optimize this case by doing a block copy
      memcpy(data, val.data, sizeof(T));
    }
    else if_constexpr_named(
      cond3, is_array_v<T> && is_fundamental_or_ptr_like_v<T_ElementType>)
    {
      // Something like an int[5], but the machine models are identical
      // or something like void*[5] which may require pointer swizzling
      // in this scenario, use a simple for loop
      //
      // TODO: remaining optimization exists. If the swizzling is a noop, also
      // use a block copy
      for (size_t i = 0; i < sizeof(T) / sizeof(T_ElementType); i++) {
        data[i] = val.data[i].get_raw_value();
      }
    }
    else
    {
      constexpr auto unknownCase = !(cond1 || cond2 || cond3);
      rlbox_detail_static_fail_because(unknownCase,
                                       "Unexpected case for assign_value");
    }
  }

public:
  tainted() = default;
  tainted(const tainted<T, T_Sandbox>& p) = default;
  tainted(const tainted_volatile<T, T_Sandbox>& p) { assign_value(p); }

  // We explicitly disable this constructor if it has one of the signatures
  // above, so that we give the above constructors a higher priority. We only
  // allow this for fundamental types as this is potentially unsafe for pointers
  // and structs
  template<typename Arg,
           typename... Args,
           RLBOX_ENABLE_IF(!std::is_base_of_v<tainted_base<T, T_Sandbox>,
                                              std::remove_reference_t<Arg>> &&
                           is_fundamental_or_enum_v<T>)>
  tainted(Arg&& arg, Args&&... args)
    : data(std::forward<Arg>(arg), std::forward<Args>(args)...)
  {}

  inline valid_return_t<T> UNSAFE_Unverified() const noexcept
  {
    return get_raw_value();
  }

  inline valid_return_t<T> UNSAFE_Sandboxed() const noexcept
  {
    return get_raw_sandbox_value();
  }
};

template<typename T, typename T_Sandbox>
class tainted_volatile : public tainted_base<T, T_Sandbox>
{
  KEEP_CLASSES_FRIENDLY

  // Classes recieve their own specialization
  static_assert(!std::is_class_v<T>,
                "Missing specialization for class T. This error occurs for one "
                "of 2 reasons.\n"
                "  1) Make sure you have include a call rlbox_load_library_api "
                "for this library.\n"
                "  2) Make sure you run (re-run) the struct-dump tool to list "
                "all structs in use by your program.\n");

private:
  using T_ConvertedType = typename T_Sandbox::template convert_sandbox_t<T>;
  T_ConvertedType data;

  inline T get_raw_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted_volatile is the type of data in sandbox memory, the
      // address of data (&data) refers to a location in sandbox memory and can
      // thus be the example_unsandboxed_ptr
      return T_Sandbox::get_unsandboxed_pointer(
        data, &data /* example_unsandboxed_ptr */);
    } else {
      return adjust_type_size<T_ConvertedType>(data);
    }
  }

  inline T_ConvertedType get_raw_sandbox_value() const noexcept
  {
    return data;
  };
};

}

// Exports
using rlbox::RLBoxSandbox;
using rlbox::tainted;
using rlbox::tainted_volatile;