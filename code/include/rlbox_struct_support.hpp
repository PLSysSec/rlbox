#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <cstring>
#include <type_traits>

#include "rlbox_assign.hpp"
#include "rlbox_types.hpp"

namespace rlbox::detail {

template<typename T, typename T_Sbx, typename T_Enable = void>
struct convert_to_sandbox_equivalent_helper;

template<typename T, typename T_Sbx>
struct convert_to_sandbox_equivalent_helper<
  T,
  T_Sbx,
  std::enable_if_t<!std::is_class_v<T>>>
{
  using type = typename RLBoxSandbox<
    T_Sbx>::template convert_to_sandbox_equivalent_nonclass_t<T>;
};

template<typename T, typename T_Sbx>
using convert_to_sandbox_equivalent_t =
  typename convert_to_sandbox_equivalent_helper<T, T_Sbx>::type;

}

#define helper_create_converted_field(fieldType, fieldName, isFrozen)          \
  typename detail::convert_to_sandbox_equivalent_t<fieldType, T_Sbx> fieldName;

#define helper_no_op()

#define sandbox_equivalent_specialization(T, libId)                            \
  template<typename T_Sbx>                                                     \
  struct Sbx_##libId##_##T                                                     \
  {                                                                            \
    sandbox_fields_reflection_##libId##_class_##T(                             \
      helper_create_converted_field,                                           \
      helper_no_op)                                                            \
  };                                                                           \
                                                                               \
  /* add convert_to_sandbox_equivalent_t specialization for new struct */      \
  namespace detail {                                                           \
    template<typename T_Template, typename T_Sbx>                              \
    struct convert_to_sandbox_equivalent_helper<                               \
      T_Template,                                                              \
      T_Sbx,                                                                   \
      std::enable_if_t<std::is_same_v<T_Template, T>>>                         \
    {                                                                          \
      using type = Sbx_##libId##_##T<T_Sbx>;                                   \
    };                                                                         \
  }

#define helper_create_tainted_field(fieldType, fieldName, isFrozen)            \
  tainted<fieldType, T_Sbx> fieldName;

#define helper_create_tainted_v_field(fieldType, fieldName, isFrozen)          \
  tainted_volatile<fieldType, T_Sbx> fieldName;

#define helper_field_wrapper_to_struct(fieldType, fieldName, isFrozen)         \
  rlbox::detail::assign_or_copy<fieldType>(lhs.fieldName,                      \
                                           rhs.fieldName.UNSAFE_Unverified());

#define helper_field_wrapper_to_sbx_struct(fieldType, fieldName, isFrozen)     \
  rlbox::detail::assign_or_copy<                                               \
    rlbox::detail::convert_to_sandbox_equivalent_t<fieldType, T_Sbx>>(         \
    lhs.fieldName, rhs.fieldName.UNSAFE_Sandboxed());

#define helper_field_wrapper_to_wrapper(fieldType, fieldName, isFrozen)        \
  lhs.fieldName = rhs.fieldName;

#define tainted_data_specialization(T, libId)                                  \
                                                                               \
  template<typename T_Sbx>                                                     \
  class tainted_volatile<T, T_Sbx>                                             \
  {                                                                            \
  private:                                                                     \
    inline Sbx_##libId##_##T<T_Sbx>& get_sandbox_value_ref() noexcept          \
    {                                                                          \
      return *reinterpret_cast<Sbx_##libId##_##T<T_Sbx>*>(this);               \
    }                                                                          \
                                                                               \
    inline T get_raw_value() const noexcept                                    \
    {                                                                          \
      T lhs;                                                                   \
      auto& rhs = *this;                                                       \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_struct, helper_no_op)                          \
                                                                               \
        return lhs;                                                            \
    }                                                                          \
                                                                               \
    /* get_raw_sandbox_value has to return a custom struct to deal with the    \
     * adjusted machine model, to ensure */                                    \
    inline Sbx_##libId##_##T<T_Sbx> get_raw_sandbox_value() const noexcept     \
    {                                                                          \
      auto ret_ptr = reinterpret_cast<const Sbx_##libId##_##T<T_Sbx>*>(this);  \
      return *ret_ptr;                                                         \
    }                                                                          \
                                                                               \
    tainted_volatile() = default;                                              \
    tainted_volatile(const tainted_volatile<T, T_Sbx>& p) = default;           \
                                                                               \
  public:                                                                      \
    sandbox_fields_reflection_##libId##_class_##T(                             \
      helper_create_tainted_v_field,                                           \
      helper_no_op)                                                            \
                                                                               \
      inline tainted<T*, T_Sbx>                                                \
      operator&() noexcept                                                     \
    {                                                                          \
      auto ref_cast = reinterpret_cast<T*>(&get_sandbox_value_ref());          \
      tainted<T*, T_Sbx> ret(ref_cast);                                        \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    /* Can't define this yet due, to mutually dependent definition between     \
    tainted and tainted_volatile for structs */                                \
    inline tainted_volatile<T, T_Sbx>& operator=(                              \
      const tainted<T, T_Sbx>&& rhs);                                          \
  };                                                                           \
                                                                               \
  template<typename T_Sbx>                                                     \
  class tainted<T, T_Sbx>                                                      \
  {                                                                            \
  private:                                                                     \
    inline T get_raw_value() const noexcept                                    \
    {                                                                          \
      auto ret_ptr = reinterpret_cast<const T*>(this);                         \
      return *ret_ptr;                                                         \
    }                                                                          \
                                                                               \
    /* get_raw_sandbox_value has to return a custom struct to deal with the    \
     * adjusted machine model, to ensure */                                    \
    inline Sbx_##libId##_##T<T_Sbx> get_raw_sandbox_value() const noexcept     \
    {                                                                          \
      Sbx_##libId##_##T<T_Sbx> lhs;                                            \
      auto& rhs = *this;                                                       \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_sbx_struct, helper_no_op)                      \
                                                                               \
        return lhs;                                                            \
    }                                                                          \
                                                                               \
  public:                                                                      \
    sandbox_fields_reflection_##libId##_class_##T(helper_create_tainted_field, \
                                                  helper_no_op)                \
                                                                               \
      tainted() = default;                                                     \
    tainted(const tainted<T, T_Sbx>& p) = default;                             \
                                                                               \
    tainted(const tainted_volatile<T, T_Sbx>& p)                               \
    {                                                                          \
      auto& lhs = *this;                                                       \
      auto& rhs = p;                                                           \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_wrapper, helper_no_op)                         \
    }                                                                          \
  };                                                                           \
                                                                               \
  /* Had to delay the definition due, to mutually dependence between           \
    tainted and tainted_volatile for structs */                                \
  template<typename T_Sbx>                                                     \
  inline tainted_volatile<T, T_Sbx>& tainted_volatile<T, T_Sbx>::operator=(    \
    const tainted<T, T_Sbx>&& rhs)                                             \
  {                                                                            \
    auto& lhs = *this;                                                         \
    sandbox_fields_reflection_##libId##_class_##T(                             \
      helper_field_wrapper_to_wrapper, helper_no_op)                           \
                                                                               \
      return *this;                                                            \
  }

// clang-format off
#define rlbox_load_structs_from_library(libId)                                 \
  namespace rlbox {                                                            \
    namespace detail {                                                         \
      struct markerStruct                                                      \
      {};                                                                      \
    }                                                                          \
    /* check that this macro is called in a global namespace */                \
    static_assert(                                                             \
      ::rlbox::detail::is_member_of_rlbox_detail<detail::markerStruct>,        \
      "Invoke rlbox_load_structs_from_library in the global namespace");                \
                                                                               \
    sandbox_fields_reflection_##libId##_allClasses(                            \
      sandbox_equivalent_specialization)                                       \
                                                                               \
    sandbox_fields_reflection_##libId##_allClasses(                            \
      tainted_data_specialization)                                                                   \
  }

// clang-format on