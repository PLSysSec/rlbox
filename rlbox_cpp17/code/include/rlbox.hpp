#pragma once

#include <type_traits>

#include "rlbox_assign.hpp"
#include "rlbox_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_types.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

////////////////

template<typename T, typename T_Sbx>
class tainted : public tainted_base<T, T_Sbx>
{
  KEEP_CLASSES_FRIENDLY
  KEEP_ASSIGNMENT_FRIENDLY

  // Classes recieve their own specialization
  static_assert(!std::is_class_v<T>,
                "Missing specialization for class T. This error occurs for one "
                "of 2 reasons.\n"
                "  1) Make sure you have include a call rlbox_load_library_api "
                "for this library.\n"
                "  2) Make sure you run (re-run) the struct-dump tool to list "
                "all structs in use by your program.\n");

private:
  using T_ConvertedType = typename T_Sbx::template convert_sandbox_t<T>;
  T data;

  inline T get_raw_value() const noexcept { return data; }

  inline T_ConvertedType get_raw_sandbox_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted<ptrs> can only be null or a pointer referring to a
      // location in sandbox memory, data can thus be the
      // example_unsandboxed_ptr
      return T_Sbx::get_sandboxed_pointer(data,
                                          data /* example_unsandboxed_ptr */);
    } else {
      return adjust_type_size<T_ConvertedType>(data);
    }
  };

public:
  tainted() = default;
  tainted(const tainted<T, T_Sbx>& p) = default;
  tainted(const tainted_volatile<T, T_Sbx>& p)
  {
    assign_wrapped_value(*this, p);
  }

  // We explicitly disable this constructor if it has one of the signatures
  // above, so that we give the above constructors a higher priority. We only
  // allow this for fundamental types as this is potentially unsafe for pointers
  // and structs
  template<typename Arg,
           typename... Args,
           RLBOX_ENABLE_IF(!std::is_base_of_v<tainted_base<T, T_Sbx>,
                                              std::remove_reference_t<Arg>> &&
                           is_fundamental_or_enum_v<T>)>
  tainted(Arg&& arg, Args&&... args)
    : data(std::forward<Arg>(arg), std::forward<Args>(args)...)
  {}

  inline valid_return_t<T> UNSAFE_Unverified() const noexcept
  {
    return get_raw_value();
  }

  inline valid_return_t<T> UNSAFE_Sandboxed() const
  {
    return get_raw_sandbox_value();
  }
};

template<typename T, typename T_Sbx>
class tainted_volatile : public tainted_base<T, T_Sbx>
{
  KEEP_CLASSES_FRIENDLY
  KEEP_ASSIGNMENT_FRIENDLY

  // Classes recieve their own specialization
  static_assert(!std::is_class_v<T>,
                "Missing specialization for class T. This error occurs for one "
                "of 2 reasons.\n"
                "  1) Make sure you have include a call rlbox_load_library_api "
                "for this library.\n"
                "  2) Make sure you run (re-run) the struct-dump tool to list "
                "all structs in use by your program.\n");

private:
  using T_ConvertedType = typename T_Sbx::template convert_sandbox_t<T>;
  T_ConvertedType data;

  inline T get_raw_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted_volatile is the type of data in sandbox memory, the
      // address of data (&data) refers to a location in sandbox memory and can
      // thus be the example_unsandboxed_ptr
      return T_Sbx::get_unsandboxed_pointer(
        data, &data /* example_unsandboxed_ptr */);
    } else {
      return adjust_type_size<T>(data);
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