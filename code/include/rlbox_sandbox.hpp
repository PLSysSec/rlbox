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
  /***** Function to adjust for custom machine models *****/

  template<typename T>
  using convert_sandbox_t = convert_base_types_t<T,
                                                 typename T_Sbx::T_IntType,
                                                 typename T_Sbx::T_LongType,
                                                 typename T_Sbx::T_LongLongType,
                                                 typename T_Sbx::T_PointerType>;

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