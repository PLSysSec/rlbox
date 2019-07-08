#pragma once

#include <cstdlib>

class rlbox_noop_sandbox
{
private:
public:
  // Stick with the system defaults
  using T_LongLongType = long long;
  using T_LongType = long;
  using T_IntType = int;
  using T_PointerType = uintptr_t;

protected:
  inline void impl_create_sandbox() {}

  inline void impl_destroy_sandbox() {}

  template<typename T>
  inline void* impl_get_unsandboxed_pointer(T_PointerType p) const
  {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(p));
  }

  template<typename T>
  inline T_PointerType impl_get_sandboxed_pointer(const void* p) const
  {
    return static_cast<T_PointerType>(reinterpret_cast<uintptr_t>(p));
  }

  template<typename T>
  static inline void* impl_get_unsandboxed_pointer(T_PointerType p, const void*)
  {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(p));
  }

  template<typename T>
  static inline T_PointerType impl_get_sandboxed_pointer(const void* p,
                                                         const void*)
  {
    return static_cast<T_PointerType>(reinterpret_cast<uintptr_t>(p));
  }

  inline T_PointerType impl_malloc_in_sandbox(size_t size)
  {
    void* p = std::malloc(size);
    return reinterpret_cast<uintptr_t>(p);
  }

  inline void impl_free_in_sandbox(T_PointerType p)
  {
    std::free(reinterpret_cast<void*>(p));
  }

  static inline bool impl_is_in_same_sandbox(const void*, const void*)
  {
    return true;
  }

  inline bool impl_is_pointer_in_sandbox_memory(const void*) { return true; }

  inline size_t impl_get_total_memory()
  {
    return std::numeric_limits<size_t>::max();
  }
};
