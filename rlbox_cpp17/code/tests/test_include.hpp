#include <cstdint>
#include <limits>
#include <type_traits>

#include "catch2/catch.hpp"

#define RLBOX_NO_COMPILE_CHECKS
#define RLBOX_USE_EXCEPTIONS
#include "rlbox.hpp"

#define UNUSED(varName) (void)varName

const uint32_t SandboxMemoryBase = 0xAF00;
using T_EmptySandbox_PointerType = uint32_t;

class EmptySandboxType
{
public:
  using T_LongLongType = int64_t;
  using T_LongType = int32_t;
  using T_IntType = int32_t;
  using T_PointerType = T_EmptySandbox_PointerType;

  static const uint32_t SandboxMemoryBaseMask = 0xFF00;

  template<typename T>
  inline const void* impl_get_unsandboxed_pointer(T_PointerType p) const
  {
    return reinterpret_cast<const void*>(SandboxMemoryBase +
                                         static_cast<uintptr_t>(p));
  }

  template<typename T>
  inline T_PointerType impl_get_sandboxed_pointer(const void* p) const
  {
    return static_cast<T_PointerType>(reinterpret_cast<uintptr_t>(p) -
                                      SandboxMemoryBase);
  }

  template<typename T>
  static inline const void* impl_get_unsandboxed_pointer(
    T_PointerType p,
    const void* example_unsandboxed_ptr)
  {
    auto mask = SandboxMemoryBaseMask &
                reinterpret_cast<uintptr_t>(example_unsandboxed_ptr);
    return reinterpret_cast<const void*>(mask + static_cast<uintptr_t>(p));
  }

  template<typename T>
  static inline T_PointerType impl_get_sandboxed_pointer(
    const void* p,
    const void* example_unsandboxed_ptr)
  {
    auto mask = SandboxMemoryBaseMask &
                reinterpret_cast<uintptr_t>(example_unsandboxed_ptr);
    return static_cast<T_PointerType>(reinterpret_cast<uintptr_t>(p) - mask);
  }
};

using T_Sbx = RLBoxSandbox<EmptySandboxType>;