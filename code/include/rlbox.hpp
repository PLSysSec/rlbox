#pragma once

#include <type_traits>

#include "rlbox_assign.hpp"
#include "rlbox_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_types.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

template<template<typename, typename> typename T_Wrap,
         typename T,
         typename T_Sbx>
class tainted_base_impl
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T>
{
  KEEP_CLASSES_FRIENDLY

private:
  inline T_Wrap<T, T_Sbx>& impl()
  {
    return *static_cast<T_Wrap<T, T_Sbx>*>(this);
  }

protected:
  using T_OpDerefRet = detail::dereference_result_t<T>;

public:
  inline auto UNSAFE_Unverified() { return impl().get_raw_value(); }
  inline auto UNSAFE_Sandboxed() { return impl().get_raw_sandbox_value(); }
};

template<typename T, typename T_Sbx>
class tainted : public tainted_base_impl<tainted, T, T_Sbx>
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

  inline detail::valid_return_t<T> get_raw_value() const noexcept
  {
    return data;
  }

  inline detail::valid_return_t<T_ConvertedType> get_raw_sandbox_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted<ptrs> can only be null or a pointer referring to a
      // location in sandbox memory, data can thus be the
      // example_unsandboxed_ptr
      return T_Sbx::get_sandboxed_pointer(data,
                                          data /* example_unsandboxed_ptr */);
    } else {
      return detail::adjust_type_size<T_ConvertedType>(data);
    }
  };

  // Initializing with a pointer is dangerous and permitted only internally
  template<typename T2 = T, RLBOX_ENABLE_IF(std::is_pointer_v<T2>)>
  tainted(T2 val)
    : data(val)
  {
    static_assert(std::is_pointer_v<T>);
  }

public:
  tainted() = default;
  tainted(tainted<T, T_Sbx>& p) = default;
  tainted(tainted_volatile<T, T_Sbx>& p)
  {
    detail::assign_wrapped_value(*this, p);
  }

  // We explicitly disable this constructor if it has one of the signatures
  // above, so that we give the above constructors a higher priority. We only
  // allow this for fundamental types as this is potentially unsafe for pointers
  // and structs
  template<typename Arg,
           typename... Args,
           RLBOX_ENABLE_IF(!std::is_base_of_v<tainted_base<T, T_Sbx>,
                                              std::remove_reference_t<Arg>> &&
                           detail::is_fundamental_or_enum_v<T>)>
  tainted(Arg&& arg, Args&&... args)
    : data(std::forward<Arg>(arg), std::forward<Args>(args)...)
  {}

private:
  using T_OpDerefRet = detail::dereference_result_t<T>;

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

  // In general comparison operators are unsafe.
  // However comparing tainted with nullptr is fine because
  // 1) tainted values are in application memory and thus cannot change the
  // value after comparision
  // 2) Checking that a pointer is null doesn't "really" taint the result as the
  // result is always safe
  inline bool operator==(const std::nullptr_t& arg) const
  {
    if_constexpr_named(cond1, std::is_pointer_v<T>)
    {
      return get_raw_value() == arg;
    }
    else
    {
      rlbox_detail_static_fail_because(
        !cond1, "Comparisons to nullptr only permitted for pointer types");
    }
  }

  inline bool operator!=(const std::nullptr_t& arg) const
  {
    if_constexpr_named(cond1, std::is_pointer_v<T>)
    {
      return get_raw_value() != arg;
    }
    else
    {
      rlbox_detail_static_fail_because(
        !cond1, "Comparisons to nullptr only permitted for pointer types");
    }
  }

  inline bool operator!()
  {
    // Technically operator ! is permitted on static arrays as well, but until
    // we figure out what that does, we do not support this as a precaution
    if_constexpr_named(cond1, std::is_pointer_v<T>)
    {
      // Checking for null pointer
      return get_raw_value() == nullptr;
    }
    else
    {
      auto unknownCase = !(cond1);
      rlbox_detail_static_fail_because(
        unknownCase, "Operator ! only permitted for pointer types");
    }
  }
};

template<typename T, typename T_Sbx>
class tainted_volatile : public tainted_base_impl<tainted_volatile, T, T_Sbx>
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

  inline detail::valid_return_t<T> get_raw_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted_volatile is the type of data in sandbox memory, the
      // address of data (&data) refers to a location in sandbox memory and can
      // thus be the example_unsandboxed_ptr
      return T_Sbx::template get_unsandboxed_pointer<std::remove_pointer_t<T>>(
        data, &data /* example_unsandboxed_ptr */);
    } else {
      return detail::adjust_type_size<T>(data);
    }
  }

  inline detail::valid_return_t<T_ConvertedType> get_raw_sandbox_value() const
    noexcept
  {
    return data;
  };

  tainted_volatile() = default;
  tainted_volatile(tainted_volatile<T, T_Sbx>& p) = default;

  using T_OpDerefRet = detail::dereference_result_t<T>;

public:
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

  inline tainted<T*, T_Sbx> operator&() noexcept
  {
    tainted<T*, T_Sbx> ret(&data);
    return ret;
  }
};

}
