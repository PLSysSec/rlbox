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

  inline valid_return_t<T> get_raw_value() const noexcept { return data; }

  inline valid_return_t<T_ConvertedType> get_raw_sandbox_value() const
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

  //Initializing with a pointer is dangerous and permitted only internally
  template<typename T2 = T, RLBOX_ENABLE_IF(std::is_pointer_v<T2>)>
  tainted(T2 val)
    : data(val)
  {
    static_assert(std::is_pointer_v<T>);
  }

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

private:
  using T_OpDerefRet = std::decay_t<std::remove_extent_t<T>>;

public:
  inline std::conditional_t<std::is_pointer_v<T>,
                            tainted_volatile<T_OpDerefRet, T_Sbx>&,
                            tainted<T_OpDerefRet, T_Sbx>* // is_array
                            >
  operator*() const
  {
    if_constexpr_named(cond1, std::is_pointer_v<T>)
    {
      auto ret_ptr = reinterpret_cast<tainted_volatile<T_OpDerefRet, T_Sbx>*>(
        get_raw_value());
      return *ret_ptr;
    }
    else if_constexpr_named(cond2, std::is_array_v<T>)
    {
      // C arrays are value types
      // Dereferencing an array in application memory returns a pointer to
      // application memory
      std::remove_extent_t<T>* decayed_arr = get_raw_value();
      auto decayed_arr_wrapped =
        reinterpret_cast<tainted<T_OpDerefRet, T_Sbx>*>(decayed_arr);
      return *decayed_arr_wrapped;
    }
    else
    {
      auto unknownCase = !(cond1 || cond2);
      rlbox_detail_static_fail_because(
        unknownCase, "Dereference only supported for pointers or arrays");
    }
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

  inline valid_return_t<T> get_raw_value() const
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

  inline valid_return_t<T_ConvertedType> get_raw_sandbox_value() const noexcept
  {
    return data;
  };

private:
  using T_OpDerefRet = std::decay_t<std::remove_extent_t<T>>;

public:
  inline valid_return_t<T> UNSAFE_Unverified() const { return get_raw_value(); }

  inline valid_return_t<T> UNSAFE_Sandboxed() const noexcept
  {
    return get_raw_sandbox_value();
  }

  inline tainted_volatile<T_OpDerefRet, T_Sbx>& operator*() const
  {
    if_constexpr_named(cond1, std::is_pointer_v<T>)
    {
      auto ret_ptr = reinterpret_cast<tainted_volatile<T_OpDerefRet, T_Sbx>*>(
        get_raw_value());
      return *ret_ptr;
    }
    else if_constexpr_named(cond2, std::is_array_v<T>)
    {
      // C arrays are value types.
      // Dereferencing an array in sandbox memory returns a pointer to sandbox
      // memory
      std::remove_extent_t<T>* decayed_arr = get_raw_value();
      auto decayed_arr_wrapped =
        reinterpret_cast<tainted_volatile<T_OpDerefRet, T_Sbx>*>(decayed_arr);
      return *decayed_arr_wrapped;
    }
    else
    {
      auto unknownCase = !(cond1 || cond2);
      rlbox_detail_static_fail_because(
        unknownCase, "Dereference only supported for pointers or arrays");
    }
  }

  inline tainted<T*, T_Sbx> operator&() const noexcept
  {
    tainted<T*, T_Sbx> ret(&data);
    return ret;
  }
};

}

// Exports
using rlbox::RLBoxSandbox;
using rlbox::tainted;
using rlbox::tainted_volatile;