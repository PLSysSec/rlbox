#pragma once

#include <type_traits>

#include "rlbox_assign.hpp"
#include "rlbox_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_struct_support.hpp"
#include "rlbox_types.hpp"
#include "rlbox_typetraits.hpp"
#include "rlbox_unwrap.hpp"

namespace rlbox {

template<template<typename, typename> typename T_Wrap,
         typename T,
         typename T_Sbx>
class tainted_base_impl
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T>
{
  KEEP_CLASSES_FRIENDLY
  KEEP_ASSIGNMENT_FRIENDLY

private:
  inline auto& impl() { return *static_cast<T_Wrap<T, T_Sbx>*>(this); }

  inline const auto& impl_c() const
  {
    return *static_cast<const T_Wrap<T, T_Sbx>*>(this);
  }

protected:
  using T_OpDerefRet = detail::dereference_result_t<T>;

public:
  inline auto UNSAFE_Unverified() const { return impl_c().get_raw_value(); }
  inline auto UNSAFE_Sandboxed() const
  {
    return impl_c().get_raw_sandbox_value();
  }

#define BinaryOp(opSymbol)                                                     \
  template<typename T_Rhs>                                                     \
  inline auto operator opSymbol(T_Rhs&& rhs)                                   \
  {                                                                            \
    static_assert(detail::is_basic_type_v<T>,                                  \
                  "Operator " #opSymbol                                        \
                  " only supported for primitive and pointer types");          \
                                                                               \
    auto raw = impl().get_raw_value();                                         \
    auto raw_rhs = detail::unwrap_value(rhs);                                  \
    static_assert(std::is_integral_v<decltype(raw_rhs)>,                       \
                  "Can only add numeric types");                               \
                                                                               \
    auto ret = raw opSymbol raw_rhs;                                           \
    using T_Ret = decltype(ret);                                               \
                                                                               \
    if constexpr (std::is_pointer_v<T_Ret>) {                                  \
      detail::dynamic_check(raw != nullptr,                                    \
                            "Pointer arithmetic on a null pointer");           \
      auto no_overflow = T_Sbx::is_in_same_sandbox(raw, ret);                  \
      detail::dynamic_check(                                                   \
        no_overflow,                                                           \
        "Pointer arithmetic overflowed a pointer beyond sandbox memory");      \
    }                                                                          \
                                                                               \
    return tainted<T_Ret, T_Sbx>(ret);                                         \
  }

  BinaryOp(+)
  BinaryOp(-)
  BinaryOp(*)
  BinaryOp(/)
  BinaryOp(%)
  BinaryOp(^)
  BinaryOp(&)
  BinaryOp(|)
  BinaryOp(<<)
  BinaryOp(>>)

#undef BinaryOp

#define UnaryOp(opSymbol)                                                      \
  inline auto operator opSymbol()                                              \
  {                                                                            \
    static_assert(detail::is_fundamental_or_enum_v<T>,                         \
                  "Operator " #opSymbol " only supported for primitive");      \
                                                                               \
    auto raw = impl().get_raw_value();                                         \
    auto ret = opSymbol raw;                                                   \
    using T_Ret = decltype(ret);                                               \
    return tainted<T_Ret, T_Sbx>(ret);                                         \
  }

  UnaryOp(-)
  UnaryOp(~)

#undef UnaryOp

  template<typename T_Rhs>
  inline tainted_volatile<detail::dereference_result_t<T>, T_Sbx>& operator[](
    T_Rhs&& rhs)
  {
    static_assert(std::is_pointer_v<T>,
                  "Operator [] in tainted_base_impl only supports pointers");

    auto ptr = impl().get_raw_value();
    auto raw_rhs = detail::unwrap_value(rhs);
    static_assert(std::is_integral_v<decltype(raw_rhs)>,
                  "Can only index with numeric types");

    auto target = &(ptr[raw_rhs]);
    auto no_overflow = T_Sbx::is_in_same_sandbox(ptr, target);
    detail::dynamic_check(
      no_overflow,
      "Pointer arithmetic overflowed a pointer beyond sandbox memory");

    auto target_wrap = tainted<detail::dereference_result_t<T>, T_Sbx>(target);
    return *target_wrap;
  }

  // We need to implement the -> operator even though T is not a struct
  // So that we can support code patterns such as the below
  // tainted<T*> a;
  // a->UNSAFE_Unverified();
  inline tainted_volatile<std::remove_pointer_t<T>, T_Sbx>* operator->()
  {
    static_assert(std::is_pointer_v<T>,
                  "Operator -> only supported for pointer types");
    auto ret = impl().get_raw_value();
    using T_Ret = std::remove_pointer_t<T>;
    return reinterpret_cast<tainted_volatile<T_Ret, T_Sbx>*>(ret);
  }

  template<typename T_Def>
  inline T_Def copy_and_verify(
    std::function<RLBox_Verify_Status(detail::valid_param_t<T>)> verifier,
    T_Def defaultValue) const
  {
    using T_Deref = std::remove_pointer_t<T>;

    if_constexpr_named(cond1, detail::is_fundamental_or_enum_v<T>)
    {
      static_assert(std::is_same_v<T_Def, T>, "Incorrect default type");
      auto val = impl_c().get_raw_value();
      return verifier(val) == RLBox_Verify_Status::SAFE ? val : defaultValue;
    }
    else if_constexpr_named(
      cond2, detail::is_one_level_ptr_v<T> && !std::is_class_v<T_Deref>)
    {
      static_assert(std::is_same_v<T_Def, std::remove_pointer_t<T>>,
                    "Incorrect default type");
      static_assert(!std::is_void_v<T_Deref>,
                    "copy_and_verify does not work for void*. Cast it to a "
                    "different tainted pointer with sandbox_reinterpret_cast");

      auto val = impl_c().get_raw_value();
      if (val == nullptr) {
        return defaultValue;
      } else {
        // Important to assign to a local variable (i.e. make a copy)
        // Else, for tainted_volatile, this will allow a
        // time-of-check-time-of-use attack
        auto val_deref = *val;
        return verifier(&val_deref) == RLBox_Verify_Status::SAFE ? val_deref
                                                                 : defaultValue;
      }
    }
    else if_constexpr_named(
      cond3, detail::is_one_level_ptr_v<T> && std::is_class_v<T_Deref>)
    {
      rlbox_detail_static_fail_because(
        cond3, "TODO: copy_and_verify not yet implemented for class pointers");
    }
    else
    {
      auto unknownCase = !(cond1 || cond2 || cond3);
      rlbox_detail_static_fail_because(
        unknownCase,
        "copy_and_verify not supported for this type as it may be unsafe");
    }
  }
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
  using T_ClassBase = tainted_base_impl<tainted, T, T_Sbx>;
  using T_ConvertedType = typename T_Sbx::template convert_sandbox_t<T>;
  T data;

  inline T& get_raw_value_ref() const noexcept { return data; }

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
  tainted(const tainted_volatile<T, T_Sbx>& p)
  {
    detail::assign_wrapped_value(*this, p);
  }
  tainted(const std::nullptr_t& arg)
    : data(arg)
  {
    static_assert(std::is_pointer_v<T>);
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

  // Needed as the definition of unary * above shadows the base's binary *
  rlbox_detail_forward_binop_to_base(*, T_ClassBase)

  // Operator [] is subtly different for tainted <static arrays> and
  // tainted_volatile<static arrays>
  template<typename T_Rhs>
  inline std::conditional_t<
    std::is_pointer_v<T>,
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>&, // is_pointer
    tainted<detail::dereference_result_t<T>, T_Sbx>&           // is_array
    >
  operator[](T_Rhs&& rhs)
  {
    static_assert(std::is_pointer_v<T> && std::is_array_v<T>,
                  "Operator [] only supported for pointers and static arrays");

    if constexpr (std::is_pointer_v<T>) {
      // defer to base class
      return (static_cast<T_ClassBase*>(this))[rhs];
    }

    // array
    auto raw_rhs = detail::unwrap_value(rhs);
    static_assert(std::is_integral_v<decltype(raw_rhs)>,
                  "Can only index with numeric types");

    detail::dynamic_check(raw_rhs >= 0 && raw_rhs < std::extent_v<T, 0>,
                          "Static array indexing overflow");

    auto& data_ref = get_raw_value_ref();
    auto target_ptr = &(data_ref[raw_rhs]);
    auto wrapped_target_ptr =
      reinterpret_cast<tainted<detail::dereference_result_t<T>, T_Sbx>*>(
        target_ptr);

    return *wrapped_target_ptr;
  }

  // In general comparison operators are unsafe.
  // However comparing tainted with nullptr is fine because
  // 1) tainted values are in application memory and thus cannot change the
  // value after comparision
  // 2) Checking that a pointer is null doesn't "really" taint the result as
  // the result is always safe
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
  using T_ClassBase = tainted_base_impl<tainted_volatile, T, T_Sbx>;
  using T_ConvertedType =
    std::add_volatile_t<typename T_Sbx::template convert_sandbox_t<T>>;
  T_ConvertedType data;

  inline T& get_raw_value_ref() const noexcept { return data; }

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

  // Needed as the definition of unary * above shadows the base's binary *
  rlbox_detail_forward_binop_to_base(*, T_ClassBase)

  inline tainted<T*, T_Sbx> operator&() noexcept
  {
    tainted<T*, T_Sbx> ret(&data);
    return ret;
  }

  // Operator [] is subtly different for tainted <static arrays> and
  // tainted_volatile<static arrays>
  template<typename T_Rhs>
  inline std::conditional_t<
    std::is_pointer_v<T>,
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>&, // is_pointer
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>&  // is_array
    >
  operator[](T_Rhs&& rhs)
  {
    static_assert(std::is_pointer_v<T> && std::is_array_v<T>,
                  "Operator [] only supported for pointers and static arrays");

    if constexpr (std::is_pointer_v<T>) {
      // defer to base class
      return (static_cast<T_ClassBase*>(this))[rhs];
    }

    // array
    static_assert(
      std::is_array_v<T>,
      "TODO: Operator [] for tainted_volatile<static arrays> is not yet "
      "implemented. Please file a bug if this is required.");
  }

  // Needed as the definition of unary & above shadows the base's binary &
  rlbox_detail_forward_binop_to_base(&, T_ClassBase)

  template<typename T_Rhs>
  inline tainted_volatile<T, T_Sbx>& operator=(T_Rhs& val)
  {
    if_constexpr_named(
      cond1, std::is_same_v<std::remove_const_t<T_Rhs>, std::nullptr_t>)
    {
      static_assert(std::is_pointer_v<T>,
                    "Null pointer can only be assigned to pointers");
      // assign using an integer instead of nullptr, as the pointer field may be
      // represented as integer
      data = 0;
    }
    else if_constexpr_named(cond2,
                            std::is_base_of_v<sandbox_wrapper_base, T_Rhs>)
    {
      detail::assign_wrapped_value(*this, val);
    }
    else if_constexpr_named(
      cond3, detail::is_fundamental_or_enum_v<T> || std::is_array_v<T>)
    {
      auto wrapped = tainted<T_Rhs, T_Sbx>(val);
      detail::assign_wrapped_value(*this, wrapped);
    }
    else if_constexpr_named(cond4, std::is_pointer_v<T_Rhs>)
    {
      rlbox_detail_static_fail_because(
        cond4,
        "Assignment of pointers is not safe as it could\n "
        "1) Leak pointers from the appliction to the sandbox\n "
        "2) Pass inaccessible pointers to the sandbox leading to crash\n "
        "3) Break sandboxes that require pointers to be swizzled first\n "
        "\n "
        "Instead, if you want to pass in a pointer, do one of the following\n "
        "1) Allocate with malloc_in_sandbox, and pass in a tainted pointer\n "
        "2) For function pointers, register with sandbox_callback, and pass in "
        "the registered value\n "
        "3) For raw pointers, use assign_pointer which performs required "
        "safety checks\n ");
    }
    else
    {
      auto unknownCase = !(cond1 || cond2 || cond3 /* || cond4 */);
      rlbox_detail_static_fail_because(
        unknownCase, "Assignment of the given type of value is not supported");
    }

    return *this;
  }
};

}
