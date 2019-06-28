#pragma once

#include <type_traits>

#include "rlbox_helpers.hpp"
#include "rlbox_typetraits.hpp"

namespace rlbox {

template<typename T_Sbx>
class RLBoxSandbox : protected T_Sbx
{
  KEEP_CLASSES_FRIENDLY

private:
  /***** Functions to adjust for custom machine models *****/

  template<class T>
  struct convert_sandbox_t_helper;

  template<>
  struct convert_sandbox_t_helper<void>
  {
    using type = void;
  };
  template<>
  struct convert_sandbox_t_helper<int>
  {
    using type = typename T_Sbx::T_IntType;
  };
  template<>
  struct convert_sandbox_t_helper<unsigned int>
  {
    using type = std::make_unsigned_t<typename T_Sbx::T_IntType>;
  };
  template<>
  struct convert_sandbox_t_helper<long>
  {
    using type = typename T_Sbx::T_LongType;
  };
  template<>
  struct convert_sandbox_t_helper<unsigned long>
  {
    using type = std::make_unsigned_t<typename T_Sbx::T_LongType>;
  };
  template<>
  struct convert_sandbox_t_helper<long long>
  {
    using type = typename T_Sbx::T_LongLongType;
  };
  template<>
  struct convert_sandbox_t_helper<unsigned long long>
  {
    using type = std::make_unsigned_t<typename T_Sbx::T_LongLongType>;
  };
  template<class T>
  struct convert_sandbox_t_helper<T*>
  {
    using type = typename T_Sbx::T_PointerType;
  };
  template<class T, std::size_t N>
  struct convert_sandbox_t_helper<T[N]>
  {
    using type = typename convert_sandbox_t_helper<T>::type[N];
  };

  template<typename T>
  using convert_sandbox_t = typename convert_sandbox_t_helper<T>::type;

public:
  template<typename T>
  inline const void* get_unsandboxed_pointer(convert_sandbox_t<T*> p) const
  {
    if (p == 0) {
      return nullptr;
    }
    return this->template impl_get_unsandboxed_pointer<T>(p);
  }

  template<typename T>
  inline convert_sandbox_t<T*> get_sandboxed_pointer(const void* p) const
  {
    if (p == nullptr) {
      return 0;
    }
    return this->template impl_get_sandboxed_pointer<T>(p);
  }

  template<typename T>
  static inline const void* get_unsandboxed_pointer(
    convert_sandbox_t<T*> p,
    const void* example_unsandboxed_ptr)
  {
    if (p == 0) {
      return nullptr;
    }
    return T_Sbx::template impl_get_unsandboxed_pointer<T>(
      p, example_unsandboxed_ptr);
  }

  template<typename T>
  static inline convert_sandbox_t<T*> get_sandboxed_pointer(
    const void* p,
    const void* example_unsandboxed_ptr)
  {
    if (p == nullptr) {
      return 0;
    }
    return T_Sbx::template impl_get_sandboxed_pointer<T>(
      p, example_unsandboxed_ptr);
  }
};

}