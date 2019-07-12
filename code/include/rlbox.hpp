#pragma once

#include <array>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "rlbox_assign.hpp"
#include "rlbox_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_policy_types.hpp"
#include "rlbox_sandbox.hpp"
#include "rlbox_stdlib.hpp"
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
  KEEP_CAST_FRIENDLY

private:
  inline auto& impl() { return *static_cast<T_Wrap<T, T_Sbx>*>(this); }
  inline auto& impl() const
  {
    return *static_cast<const T_Wrap<T, T_Sbx>*>(this);
  }

protected:
  using T_OpDerefRet = detail::dereference_result_t<T>;

public:
  inline auto UNSAFE_Unverified() { return impl().get_raw_value(); }
  inline auto UNSAFE_Sandboxed() { return impl().get_raw_sandbox_value(); }
  inline auto UNSAFE_Unverified() const { return impl().get_raw_value(); }
  inline auto UNSAFE_Sandboxed() const
  {
    return impl().get_raw_sandbox_value();
  }

#define BinaryOpValAndPtr(opSymbol)                                            \
  template<typename T_Rhs>                                                     \
  inline auto operator opSymbol(T_Rhs&& rhs)                                   \
  {                                                                            \
    static_assert(detail::is_basic_type_v<T>,                                  \
                  "Operator " #opSymbol                                        \
                  " only supported for primitive and pointer types");          \
                                                                               \
    auto raw_rhs = detail::unwrap_value(rhs);                                  \
    static_assert(std::is_integral_v<decltype(raw_rhs)>,                       \
                  "Can only operate on numeric types");                        \
                                                                               \
    if constexpr (std::is_pointer_v<T>) {                                      \
      auto ptr = impl().get_raw_value();                                       \
      detail::dynamic_check(ptr != nullptr,                                    \
                            "Pointer arithmetic on a null pointer");           \
      /* increment the target by size of the data structure */                 \
      auto target =                                                            \
        reinterpret_cast<uintptr_t>(ptr) opSymbol raw_rhs * sizeof(*impl());   \
      auto no_overflow = RLBoxSandbox<T_Sbx>::is_in_same_sandbox(              \
        reinterpret_cast<const void*>(ptr),                                    \
        reinterpret_cast<const void*>(target));                                \
      detail::dynamic_check(                                                   \
        no_overflow,                                                           \
        "Pointer arithmetic overflowed a pointer beyond sandbox memory");      \
                                                                               \
      return tainted<T, T_Sbx>(reinterpret_cast<T>(target));                   \
    } else {                                                                   \
      auto raw = impl().get_raw_value();                                       \
      auto ret = raw opSymbol raw_rhs;                                         \
      using T_Ret = decltype(ret);                                             \
      return tainted<T_Ret, T_Sbx>(ret);                                       \
    }                                                                          \
  }

  BinaryOpValAndPtr(+)
  BinaryOpValAndPtr(-)

#undef BinaryOpValAndPtr

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
                  "Can only operate on numeric types");                        \
                                                                               \
    auto ret = raw opSymbol raw_rhs;                                           \
    using T_Ret = decltype(ret);                                               \
    return tainted<T_Ret, T_Sbx>(ret);                                         \
  }

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

  using T_OpSubscriptArrRet =
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>;

  template<typename T_Rhs>
  inline const T_OpSubscriptArrRet& operator[](T_Rhs&& rhs) const
  {
    static_assert(std::is_pointer_v<T>,
                  "Operator [] in tainted_base_impl only supports pointers");

    auto ptr = this->impl().get_raw_value();
    auto raw_rhs = detail::unwrap_value(rhs);
    static_assert(std::is_integral_v<decltype(raw_rhs)>,
                  "Can only index with numeric types");

    // increment the target by size of the data structure
    auto target =
      reinterpret_cast<uintptr_t>(ptr) + raw_rhs * sizeof(*this->impl());
    auto no_overflow = RLBoxSandbox<T_Sbx>::is_in_same_sandbox(
      ptr, reinterpret_cast<const void*>(target));
    detail::dynamic_check(
      no_overflow,
      "Pointer arithmetic overflowed a pointer beyond sandbox memory");

    auto target_wrap =
      tainted<const T, T_Sbx>(reinterpret_cast<const T>(target));
    return *target_wrap;
  }

  template<typename T_Rhs>
  inline T_OpSubscriptArrRet& operator[](T_Rhs&& rhs)
  {
    rlbox_detail_forward_to_const_a(operator[], T_OpSubscriptArrRet&, rhs);
  }

  // We need to implement the -> operator even though T is not a struct
  // So that we can support code patterns such as the below
  // tainted<T*> a;
  // a->UNSAFE_Unverified();
  inline auto operator-> () const
  {
    static_assert(std::is_pointer_v<T>,
                  "Operator -> only supported for pointer types");
    auto ret = impl().get_raw_value();
    using T_Ret = std::remove_pointer_t<T>;
    using T_RetWrap = const tainted_volatile<T_Ret, T_Sbx>;
    return reinterpret_cast<T_RetWrap*>(ret);
  }

  inline auto operator-> ()
  {
    using T_Ret = tainted_volatile<std::remove_pointer_t<T>, T_Sbx>*;
    rlbox_detail_forward_to_const(operator->, T_Ret);
  }

  template<typename T_Def>
  inline T_Def copy_and_verify(
    std::function<RLBox_Verify_Status(detail::valid_param_t<T>)> verifier,
    T_Def default_val) const
  {
    using T_Deref = std::remove_pointer_t<T>;

    if_constexpr_named(cond1, detail::is_fundamental_or_enum_v<T>)
    {
      static_assert(std::is_same_v<T_Def, T>, "Incorrect default type");
      auto val = impl().get_raw_value();
      return verifier(val) == RLBox_Verify_Status::SAFE ? val : default_val;
    }
    else if_constexpr_named(
      cond2, detail::is_one_level_ptr_v<T> && !std::is_class_v<T_Deref>)
    {
      static_assert(std::is_same_v<T_Def, std::remove_pointer_t<T>>,
                    "Incorrect default type");
      static_assert(!std::is_void_v<T_Deref>,
                    "copy_and_verify does not work for void*. Cast it to a "
                    "different tainted pointer with sandbox_reinterpret_cast");

      auto val = impl().get_raw_value();
      if (val == nullptr) {
        return default_val;
      } else {
        // Important to assign to a local variable (i.e. make a copy)
        // Else, for tainted_volatile, this will allow a
        // time-of-check-time-of-use attack
        auto val_deref = *val;
        return verifier(&val_deref) == RLBox_Verify_Status::SAFE ? val_deref
                                                                 : default_val;
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

  using T_copy_verify_arr_result =
    std::array<std::remove_cv_t<std::remove_extent_t<T>>, std::extent_v<T>>;

  inline T_copy_verify_arr_result copy_and_verify_array(
    std::function<RLBox_Verify_Status(T_copy_verify_arr_result)> verifier,
    T_copy_verify_arr_result default_val) const
  {
    static_assert(std::is_array_v<T>,
                  "Can only call copy_and_verify_array on arrays");

    tainted<T, T_Sbx>* tainted_ptr;

    if constexpr (std::is_same_v<tainted<T, T_Sbx>, T_Wrap<T, T_Sbx>>) {
      tainted_ptr = &impl();
    } else {
      tainted<std::remove_cv_t<T>, T_Sbx> copy;
      detail::assign_wrapped_value_nonclass(copy, impl());
      tainted_ptr = &copy;
    }

    // This is ok as...
    // 1) tainted has the same layout as an unwrapped type
    // 2) std::array has the same layout as a T[]
    auto ret_ptr = reinterpret_cast<T_copy_verify_arr_result*>(tainted_ptr);
    return verifier(*ret_ptr) == RLBox_Verify_Status::SAFE ? *ret_ptr
                                                           : default_val;
  }

  inline detail::valid_return_t<T> copy_and_verify_range(
    std::function<RLBox_Verify_Status(detail::valid_param_t<T>)> verifier,
    std::size_t count,
    detail::valid_param_t<T> default_val) const
  {
    static_assert(std::is_pointer_v<T>,
                  "Can only call copy_and_verify_range on pointers");
    static_assert(
      detail::is_fundamental_or_enum_v<
        std::remove_cv_t<std::remove_pointer_t<T>>>,
      "copy_and_verify_range only allows fundamental types or enums");

    auto start = reinterpret_cast<const void*>(impl().get_raw_value());
    if (start == nullptr) {
      return default_val;
    }

    auto end_exclusive_tainted = &(impl()[count + 1]);
    auto end_exclusive =
      reinterpret_cast<uintptr_t>(end_exclusive_tainted.get_raw_value());
    auto end = reinterpret_cast<const void*>(end_exclusive - 1);

    auto no_overflow = RLBoxSandbox<T_Sbx>::is_in_same_sandbox(start, end);
    detail::dynamic_check(
      no_overflow,
      "Pointer arithmetic overflowed a pointer beyond sandbox memory");

    using T_El = std::remove_cv_t<std::remove_pointer_t<T>>;
    auto target = new T_El[count];

    for (size_t i = 0; i < count; i++) {
      auto tainted_ptr = reinterpret_cast<tainted<T_El, T_Sbx>*>(&(target[i]));
      detail::assign_wrapped_value_nonclass(*tainted_ptr, impl()[i]);
    }

    return verifier(target) == RLBox_Verify_Status::SAFE ? target : default_val;
  }

  inline detail::valid_return_t<T> copy_and_verify_string(
    std::function<RLBox_Verify_Status(detail::valid_param_t<T>)> verifier,
    detail::valid_param_t<T> default_val) const
  {
    static_assert(std::is_pointer_v<T>,
                  "Can only call copy_and_verify_string on pointers");

    static_assert(
      std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>>,
      "copy_and_verify_string only allows char*");

    auto start = impl().get_raw_value();
    if (start == nullptr) {
      return default_val;
    }

    // it is safe to run strlen on a tainted<string> as worst case, the string
    // does not have a null and we try to copy all the memory out of the sandbox
    // however, copy_and_verify_range ensures that we never copy memory outsider
    // the range
    auto str_len = std::strlen(start) + 1;
    auto ret = copy_and_verify_range(verifier, str_len, default_val);

    // ensure the string has a trailing null
    ret[str_len - 1] = '\0';
    return ret;
  }
};

template<typename T, typename T_Sbx>
class tainted : public tainted_base_impl<tainted, T, T_Sbx>
{
  KEEP_CLASSES_FRIENDLY
  KEEP_ASSIGNMENT_FRIENDLY
  KEEP_CAST_FRIENDLY

  // Classes recieve their own specialization
  static_assert(
    !std::is_class_v<T>,
    "Missing specialization for class T. This error occurs for one "
    "of 2 reasons.\n"
    "  1) Make sure you have include a call rlbox_load_structs_from_library "
    "for this library.\n"
    "  2) Make sure you run (re-run) the struct-dump tool to list "
    "all structs in use by your program.\n");

private:
  using T_ClassBase = tainted_base_impl<tainted, T, T_Sbx>;
  using T_ConvertedType = typename RLBoxSandbox<
    T_Sbx>::template convert_to_sandbox_equivalent_nonclass_t<T>;
  T data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

  inline detail::valid_return_t<const T> get_raw_value() const noexcept
  {
    return data;
  }

  inline detail::valid_return_t<const T_ConvertedType> get_raw_sandbox_value()
    const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Need a reinterpret_cast as function pointers don't decay
      auto data_ptr = reinterpret_cast<const void*>(data);
      // Since tainted<ptrs> can only be null or a pointer referring to a
      // location in sandbox memory, data can thus be the
      // example_unsandboxed_ptr
      return RLBoxSandbox<T_Sbx>::template get_sandboxed_pointer<T>(
        data_ptr, data_ptr /* example_unsandboxed_ptr */);
    } else {
      return detail::adjust_type_size<T_ConvertedType>(data);
    }
  };

  inline detail::valid_return_t<T> get_raw_value() noexcept
  {
    rlbox_detail_forward_to_const(get_raw_value, detail::valid_return_t<T>);
  }

  inline detail::valid_return_t<T_ConvertedType> get_raw_sandbox_value()
  {
    rlbox_detail_forward_to_const(get_raw_sandbox_value,
                                  detail::valid_return_t<T_ConvertedType>);
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
  tainted(const tainted<T, T_Sbx>& p) = default;
  tainted(const tainted_volatile<T, T_Sbx>& p)
  {
    detail::assign_wrapped_value_nonclass(*this, p);
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
  template<typename T_Arg,
           RLBOX_ENABLE_IF(
             !std::is_base_of_v<tainted_base<T, T_Sbx>,
                                std::remove_reference_t<T_Arg>> &&
             detail::is_fundamental_or_enum_v<T> &&
             detail::is_fundamental_or_enum_v<std::remove_reference_t<T_Arg>>)>
  tainted(T_Arg&& arg)
    : data(std::forward<T_Arg>(arg))
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
  using T_OpSubscriptRet = std::conditional_t<
    std::is_pointer_v<T>,
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>, // is_pointer
    tainted<detail::dereference_result_t<T>, T_Sbx>           // is_array
    >;

  template<typename T_Rhs>
  inline const T_OpSubscriptRet& operator[](T_Rhs&& rhs) const
  {
    static_assert(std::is_pointer_v<T> || std::is_array_v<T>,
                  "Operator [] only supported for pointers and static arrays");

    if constexpr (std::is_pointer_v<T>) {
      // defer to base class
      using T_Base = const T_ClassBase;
      return this->T_Base::operator[](rhs);
    } else {
      // array
      auto raw_rhs = detail::unwrap_value(rhs);
      static_assert(std::is_integral_v<decltype(raw_rhs)>,
                    "Can only index with numeric types");

      using T_Rhs_Unsigned = std::make_unsigned_t<decltype(raw_rhs)>;
      detail::dynamic_check(raw_rhs >= 0 && static_cast<T_Rhs_Unsigned>(
                                              raw_rhs) < std::extent_v<T, 0>,
                            "Static array indexing overflow");

      auto& data_ref = this->get_raw_value_ref();
      auto target_ptr = &(data_ref[raw_rhs]);
      using T_Target = const tainted<detail::dereference_result_t<T>, T_Sbx>;
      auto wrapped_target_ptr = reinterpret_cast<T_Target*>(target_ptr);

      return *wrapped_target_ptr;
    }
  }

  template<typename T_Rhs>
  inline T_OpSubscriptRet& operator[](T_Rhs&& rhs)
  {
    rlbox_detail_forward_to_const_a(operator[], T_OpSubscriptRet&, rhs);
  }

  // In general comparison operators are unsafe.
  // However comparing tainted with nullptr is fine because
  // 1) tainted values are in application memory and thus cannot change the
  // value after comparision
  // 2) Checking that a pointer is null doesn't "really" taint the result as
  // the result is always safe
  template<typename T_Rhs>
  inline bool operator==(T_Rhs&& arg) const
  {
    if_constexpr_named(
      cond1,
      !std::is_same_v<std::remove_const_t<std::remove_reference_t<T_Rhs>>,
                      std::nullptr_t>)
    {
      rlbox_detail_static_fail_because(
        cond1,
        "Only comparisons to nullptr are allowed. All other comparisons to "
        "tainted types create many antipatterns. Rather than comparing tainted "
        "values directly, unwrap the values with the copy_and_verify API and "
        "then perform the comparisons.");
    }
    else if_constexpr_named(cond2, std::is_pointer_v<T>)
    {
      return get_raw_value() == arg;
    }
    else
    {
      rlbox_detail_static_fail_because(
        !cond2, "Comparisons to nullptr only permitted for pointer types");
    }
  }

  template<typename T_Rhs>
  inline bool operator!=(T_Rhs&& arg) const
  {
    if_constexpr_named(
      cond1,
      !std::is_same_v<std::remove_const_t<std::remove_reference_t<T_Rhs>>,
                      std::nullptr_t>)
    {
      rlbox_detail_static_fail_because(
        cond1,
        "Only comparisons to nullptr are allowed. All other comparisons to "
        "tainted types create many antipatterns. Rather than comparing tainted "
        "values directly, unwrap the values with the copy_and_verify API and "
        "then perform the comparisons.");
    }
    else if_constexpr_named(cond2, std::is_pointer_v<T>)
    {
      return get_raw_value() != arg;
    }
    else
    {
      rlbox_detail_static_fail_because(
        !cond2, "Comparisons to nullptr only permitted for pointer types");
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
        unknownCase,
        "Operator ! only permitted for pointer types. For other types, unwrap "
        "the tainted value with the copy_and_verify API and then use operator "
        "!");
    }
  }
};

template<typename T, typename T_Sbx>
class tainted_volatile : public tainted_base_impl<tainted_volatile, T, T_Sbx>
{
  KEEP_CLASSES_FRIENDLY
  KEEP_ASSIGNMENT_FRIENDLY
  KEEP_CAST_FRIENDLY

  // Classes recieve their own specialization
  static_assert(
    !std::is_class_v<T>,
    "Missing specialization for class T. This error occurs for one "
    "of 2 reasons.\n"
    "  1) Make sure you have include a call rlbox_load_structs_from_library "
    "for this library.\n"
    "  2) Make sure you run (re-run) the struct-dump tool to list "
    "all structs in use by your program.\n");

private:
  using T_ClassBase = tainted_base_impl<tainted_volatile, T, T_Sbx>;
  using T_ConvertedType = std::add_volatile_t<typename RLBoxSandbox<
    T_Sbx>::template convert_to_sandbox_equivalent_nonclass_t<T>>;
  T_ConvertedType data;

  inline auto& get_sandbox_value_ref() noexcept { return data; }
  inline auto& get_sandbox_value_ref() const noexcept { return data; }

  inline detail::valid_return_t<const T> get_raw_value() const
  {
    if constexpr (std::is_pointer_v<T>) {
      // Since tainted_volatile is the type of data in sandbox memory, the
      // address of data (&data) refers to a location in sandbox memory and can
      // thus be the example_unsandboxed_ptr
      const volatile void* data_ref = &data;
      return RLBoxSandbox<T_Sbx>::template get_unsandboxed_pointer<
        std::remove_pointer_t<T>>(
        data, const_cast<const void*>(data_ref) /* example_unsandboxed_ptr */);
    } else {
      return detail::adjust_type_size<T>(data);
    }
  }

  inline detail::valid_return_t<const T_ConvertedType> get_raw_sandbox_value()
    const noexcept
  {
    return data;
  };

  inline detail::valid_return_t<T> get_raw_value()
  {
    rlbox_detail_forward_to_const(get_raw_value, detail::valid_return_t<T>);
  }

  inline detail::valid_return_t<T_ConvertedType>
  get_raw_sandbox_value() noexcept
  {
    rlbox_detail_forward_to_const(get_raw_sandbox_value,
                                  detail::valid_return_t<T_ConvertedType>);
  };

  tainted_volatile() = default;
  tainted_volatile(const tainted_volatile<T, T_Sbx>& p) = default;

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

  inline tainted<const T*, T_Sbx> operator&() const noexcept
  {
    auto ref =
      detail::remove_volatile_from_ptr_cast(&this->get_sandbox_value_ref());
    auto ref_cast = reinterpret_cast<const T*>(ref);
    tainted<const T*, T_Sbx> ret(ref_cast);
    return ret;
  }

  inline tainted<T*, T_Sbx> operator&() noexcept
  {
    rlbox_detail_forward_to_const(operator[], T_OpSubscriptRet&);
  }

  // Needed as the definition of unary & above shadows the base's binary &
  rlbox_detail_forward_binop_to_base(&, T_ClassBase)

  // Operator [] is subtly different for tainted <static arrays> and
  // tainted_volatile<static arrays>
  using T_OpSubscriptRet =
    tainted_volatile<detail::dereference_result_t<T>, T_Sbx>;

  template<typename T_Rhs>
  inline const T_OpSubscriptRet& operator[](T_Rhs&& rhs) const
  {
    static_assert(std::is_pointer_v<T> || std::is_array_v<T>,
                  "Operator [] only supported for pointers and static arrays");

    if constexpr (std::is_pointer_v<T>) {
      // defer to base class
      using T_Base = const T_ClassBase;
      return this->T_Base::operator[](rhs);
    } else {
      // array
      auto raw_rhs = detail::unwrap_value(rhs);
      static_assert(std::is_integral_v<decltype(raw_rhs)>,
                    "Can only index with numeric types");

      using T_Rhs_Unsigned = std::make_unsigned_t<decltype(raw_rhs)>;
      detail::dynamic_check(raw_rhs >= 0 && static_cast<T_Rhs_Unsigned>(
                                              raw_rhs) < std::extent_v<T, 0>,
                            "Static array indexing overflow");

      auto& data_ref = this->get_sandbox_value_ref();
      auto target_ptr = &(data_ref[raw_rhs]);
      // target_ptr points to a volatile, remove this. Safe as we will
      // ultimately return a tainted_volatile
      auto target_ptr_non_vol =
        detail::remove_volatile_from_ptr_cast(target_ptr);

      using T_Target =
        const tainted_volatile<detail::dereference_result_t<T>, T_Sbx>;
      auto wrapped_target_ptr = reinterpret_cast<T_Target*>(target_ptr_non_vol);

      return *wrapped_target_ptr;
    }
  }

  template<typename T_Rhs>
  inline T_OpSubscriptRet& operator[](T_Rhs&& rhs)
  {
    rlbox_detail_forward_to_const_a(operator[], T_OpSubscriptRet&, rhs);
  }

  template<typename T_RhsRef>
  inline tainted_volatile<T, T_Sbx>& operator=(T_RhsRef&& val)
  {
    using T_Rhs = std::remove_reference_t<T_RhsRef>;

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
      detail::assign_wrapped_value_nonclass(*this, val);
    }
    else if_constexpr_named(
      cond3, detail::is_fundamental_or_enum_v<T> || std::is_array_v<T>)
    {
      auto wrapped = tainted<T_Rhs, T_Sbx>(val);
      detail::assign_wrapped_value_nonclass(*this, wrapped);
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

  // ==, != and ! are not supported for tainted_volatile, however, we implement
  // this to ensure the user doesn't see a confusing error message
  template<typename T_Rhs>
  inline bool operator==(T_Rhs&&) const
  {
    rlbox_detail_static_fail_because(
      detail::true_v<T_Rhs>,
      "Cannot compare values that are located in sandbox memory. This error "
      "occurs if you compare a dereferenced value such as the code shown "
      "below\n\n"
      "tainted<int**> a = ...;\n"
      "assert(*a == nullptr);\n\n"
      "Instead you can write this code as \n"
      "tainted<int*> temp = *a;\n"
      "assert(temp == nullptr);\n");
    return false;
  }

  template<typename T_Rhs>
  inline bool operator!=(const std::nullptr_t&) const
  {
    rlbox_detail_static_fail_because(
      detail::true_v<T_Rhs>,
      "Cannot compare values that are located in sandbox memory. This error "
      "occurs if you compare a dereferenced value such as the code shown "
      "below\n\n"
      "tainted<int**> a = ...;\n"
      "assert(*a != nullptr);\n\n"
      "Instead you can write this code as \n"
      "tainted<int*> temp = *a;\n"
      "assert(temp != nullptr);\n");
    return false;
  }

  template<typename T_Dummy = void>
  inline bool operator!()
  {
    rlbox_detail_static_fail_because(
      detail::true_v<T_Dummy>,
      "Cannot apply 'operator not' on values that are located in sandbox "
      "memory. This error occurs if you compare a dereferenced value such as "
      "the code shown below\n\n"
      "tainted<int**> a = ...;\n"
      "assert(!(*a));\n\n"
      "Instead you can write this code as \n"
      "tainted<int*> temp = *a;\n"
      "assert(!temp);\n");
    return false;
  }
};

}
