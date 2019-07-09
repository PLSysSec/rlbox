#pragma once

#include <cstdlib>

#include "rlbox_helpers.hpp"

namespace rlbox {

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

  // adding a template so that we can use static_assert to fire only if this
  // function is invoked
  template<typename T = void>
  void* impl_lookup_symbol(const char* /* func_name */)
  {
    // Will fire if this impl_lookup_symbol is ever called for the static
    // sandbox
    constexpr bool fail = std::is_same_v<T, void>;
    rlbox_detail_static_fail_because(
      fail,
      "The no_op_sandbox uses static calls and thus developers should add\n\n"
      "#define RLBOX_USE_STATIC_CALLS rlbox_no_op_sandbox_lookup_symbol\n\n"
      "to their code, to ensure that static calls are handled correctly.");

    return nullptr;
  }

#define rlbox_no_op_sandbox_lookup_symbol(sandbox, func_name)                  \
  []() {                                                                       \
    static_assert(                                                             \
      std::is_same_v<std::remove_reference_t<decltype(sandbox)>,               \
                     rlbox::RLBoxSandbox<rlbox::rlbox_noop_sandbox>>,          \
      "Forwarding another sandboxes calls to rlbox_no_op_sandbox. "            \
      "Please check the use of RLBOX_USE_STATIC_CALLS.");                      \
    return reinterpret_cast<void*>(&func_name);                                \
  }()

  template<typename T, typename... T_Args>
  auto impl_invoke_with_func_ptr(T* func_ptr, T_Args&&... params)
  {
    return (*func_ptr)(params...);
  }
};

}