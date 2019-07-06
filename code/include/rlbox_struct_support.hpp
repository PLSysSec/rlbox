#pragma once

#include <cstring>
#include <type_traits>

#include "rlbox_types.hpp"

namespace rlbox::detail {

template<typename T_Lhs, typename T_Rhs>
void assign_or_copy(T_Lhs& lhs, T_Rhs&& rhs)
{
  if constexpr (std::is_assignable_v<T_Lhs&, T_Rhs>) {
    lhs = rhs;
  } else {
    // Use memcpy as types like static arrays are not assignable with =
    static_assert(sizeof(T_Lhs) == sizeof(T_Rhs),
                  "Unexpected size difference in tainted struct handling. "
                  "Please file a bug.");
    auto dest = reinterpret_cast<void*>(&lhs);
    auto src = reinterpret_cast<const void*>(&rhs);
    std::memcpy(dest, src, sizeof(lhs));
  }
}

template<typename T, typename T_Sbx, typename T_Enable = void>
struct convert_to_sandbox_equivalent_helper;

template<typename T, typename T_Sbx>
struct convert_to_sandbox_equivalent_helper<
  T,
  T_Sbx,
  std::enable_if_t<!std::is_class_v<T>>>
{
  using type = typename RLBoxSandbox<T_Sbx>::template convert_sandbox_t<T>;
};

template<typename T, typename T_Sbx>
using convert_to_sandbox_equivalent =
  typename convert_to_sandbox_equivalent_helper<T, T_Sbx>::type;

}

#define helper_create_converted_field(fieldType, fieldName, isFrozen, T_Sbx)   \
  typename detail::convert_to_sandbox_equivalent<fieldType, T_Sbx> fieldName;

#define helper_no_op()

#define sandbox_equivalent_specialization(T, libId, T_Sbx)                     \
  struct T_Sbx_##libId##_##T                                                   \
  {                                                                            \
    sandbox_fields_reflection_##libId##_class_##T(                             \
      helper_create_converted_field,                                           \
      helper_no_op,                                                            \
      T_Sbx)                                                                   \
  };                                                                           \
                                                                               \
  /* add convert_to_sandbox_equivalent specialization for new struct */        \
  namespace detail {                                                           \
    template<typename T_Template, typename T_Sbx>                              \
    struct convert_to_sandbox_equivalent_helper<                               \
      T_Template,                                                              \
      T_Sbx,                                                                   \
      std::enable_if_t<std::is_same_v<T_Template, T>>>                         \
    {                                                                          \
      using type = T_Sbx_##libId##_##T;                                        \
    };                                                                         \
  }

#define helper_create_tainted_field(fieldType, fieldName, isFrozen, T_Sbx)     \
  tainted<fieldType, T_Sbx> fieldName;

#define helper_create_tainted_v_field(fieldType, fieldName, isFrozen, T_Sbx)   \
  tainted_volatile<fieldType, T_Sbx> fieldName;

#define helper_field_wrapper_to_struct(fieldType, fieldName, isFrozen, T_Sbx)  \
  rlbox::detail::assign_or_copy(lhs.fieldName,                                 \
                                rhs.fieldName.UNSAFE_Unverified());

#define helper_field_wrapper_to_sbx_struct(                                    \
  fieldType, fieldName, isFrozen, T_Sbx)                                       \
  rlbox::detail::assign_or_copy(lhs.fieldName,                                 \
                                rhs.fieldName.UNSAFE_Sandboxed());

#define helper_field_wrapper_to_wrapper(fieldType, fieldName, isFrozen, T_Sbx) \
  lhs.fieldName = rhs.fieldName;

#define helper_fieldCopyUnsandbox(fieldType, fieldName, isFrozen, T_Sbx)       \
  {                                                                            \
    auto temp = fieldName.get_raw_sandbox_value(sandbox);                      \
    std::memcpy((void*)&(ret.fieldName), (void*)&temp, sizeof(ret.fieldName)); \
  }

#define helper_fieldUnsandbox(fieldType, fieldName, isFrozen, T_Sbx)           \
  fieldName.unsandboxPointersOrNull(sandbox);

#define tainted_data_specialization(T, libId, T_Sbx)                           \
                                                                               \
  template<>                                                                   \
  class tainted_volatile<T, T_Sbx>                                             \
  {                                                                            \
  private:                                                                     \
    inline T get_raw_value() const noexcept                                    \
    {                                                                          \
      T lhs;                                                                   \
      auto& rhs = *this;                                                       \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_struct, helper_no_op, T_Sbx)                   \
                                                                               \
        return lhs;                                                            \
    }                                                                          \
                                                                               \
    /* get_raw_sandbox_value has to return a custom struct to deal with the    \
     * adjusted machine model, to ensure */                                    \
    inline T_Sbx_##libId##_##T get_raw_sandbox_value(                          \
      RLBoxSandbox<T_Sbx>* sandbox) const noexcept                             \
    {                                                                          \
      auto ret_ptr = reinterpret_cast<const T_Sbx_##libId##_##T*>(this);       \
      return *ret_ptr;                                                         \
    }                                                                          \
                                                                               \
    tainted_volatile() = default;                                              \
    tainted_volatile(tainted_volatile<T, T_Sbx>& p) = default;                 \
                                                                               \
  public:                                                                      \
    sandbox_fields_reflection_##libId##_class_##T(                             \
      helper_create_tainted_v_field,                                           \
      helper_no_op,                                                            \
      T_Sbx)                                                                   \
                                                                               \
      inline tainted<T*, T_Sbx>                                                \
      operator&() noexcept                                                     \
    {                                                                          \
      tainted<T*, T_Sbx> ret(&data);                                           \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    inline tainted_volatile<T, T_Sbx>& operator=(const tainted<T, T_Sbx>& rhs) \
    {                                                                          \
      auto& lhs = *this;                                                       \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_wrapper, helper_no_op, T_Sbx)                  \
                                                                               \
        return *this;                                                          \
    }                                                                          \
  };                                                                           \
                                                                               \
  template<>                                                                   \
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
    inline T_Sbx_##libId##_##T get_raw_sandbox_value(                          \
      RLBoxSandbox<T_Sbx>* sandbox) const noexcept                             \
    {                                                                          \
      T_Sbx_##libId##_##T lhs;                                                 \
      auto& rhs = *this;                                                       \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_sbx_struct, helper_no_op, T_Sbx)               \
                                                                               \
        return lhs;                                                            \
    }                                                                          \
                                                                               \
  public:                                                                      \
    sandbox_fields_reflection_##libId##_class_##T(helper_create_tainted_field, \
                                                  helper_no_op,                \
                                                  T_Sbx)                       \
                                                                               \
      tainted() = default;                                                     \
    tainted(const tainted<T, T_Sbx>& p) = default;                             \
                                                                               \
    tainted(const tainted_volatile<T, T_Sbx>& p)                               \
    {                                                                          \
      auto& lhs = *this;                                                       \
      auto& rhs = p;                                                           \
      sandbox_fields_reflection_##libId##_class_##T(                           \
        helper_field_wrapper_to_wrapper, helper_no_op, T_Sbx)                  \
    }                                                                          \
  };

// clang-format off
#define rlbox_load_library_api(libId, T_Sbx)                                   \
  namespace rlbox {                                                            \
    namespace detail {                                                         \
      struct markerStruct                                                      \
      {};                                                                      \
    }                                                                          \
    /* check that this macro is called in a global namespace */                \
    static_assert(                                                             \
      ::rlbox::detail::is_member_of_rlbox_detail<detail::markerStruct>,        \
      "Invoke rlbox_load_library_api in the global namespace");                \
                                                                               \
    sandbox_fields_reflection_##libId##_allClasses(                            \
      sandbox_equivalent_specialization,                                       \
      T_Sbx)                                                                   \
                                                                               \
    sandbox_fields_reflection_##libId##_allClasses(                            \
      tainted_data_specialization,                                             \
      T_Sbx)                                                                   \
  }

// clang-format on