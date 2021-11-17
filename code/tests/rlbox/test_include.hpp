#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

// Convert rlbox's compile time errors to exceptions throws for easy testing
#define RLBOX_NO_COMPILE_CHECKS
#define RLBOX_USE_EXCEPTIONS
#define RLBOX_ENABLE_DEBUG_ASSERTIONS
#define RLBOX_SINGLE_THREADED_INVOCATIONS
#include "rlbox.hpp"
#include "rlbox_noop_sandbox.hpp"

// IWYU pragma: end_exports

// Since we are convert static errors to exceptions, we can now test this easily
#define REQUIRE_COMPILE_ERR REQUIRE_THROWS
#define REQUIRE_NO_COMPILE_ERR REQUIRE_NOTHROW

#define UNUSED(...) (void)__VA_ARGS__

// The point here is to use the C types as is, so turn off the links
using CallbackType = int (*)(unsigned, const char*, unsigned[1]); // NOLINT

enum testBasicEnum
{
  testBasicEnumVal1,
  testBasicEnumVal2
};

struct testVarietyStruct
{
  unsigned long fieldLong;                               // NOLINT
  const char* fieldString;                               // NOLINT
  unsigned int fieldBool;                                // NOLINT
  char fieldFixedArr[8];                                 // NOLINT
  int (*fieldFnPtr)(unsigned, const char*, unsigned[1]); // NOLINT
  struct unknownClass* fieldUnknownPtr;                  // NOLINT
  void* voidPtr;                                         // NOLINT
  CallbackType fnArray[8];                               // NOLINT
};

namespace rlbox {
class rlbox_test_sandbox;
}

using TestSandbox = rlbox::rlbox_test_sandbox;

namespace rlbox {
class rlbox_test_sandbox
{
private:
  std::pair<std::byte*, std::byte*> pow2SizeAlignedMalloc(size_t size)
  {
    size_t paddedSize = size * 2 + 1;
    std::byte* mem = new std::byte[paddedSize];
    uintptr_t memU = reinterpret_cast<uintptr_t>(mem);
    if ((memU & size) == 0) {
      return std::make_pair(mem, mem);
    }

    uintptr_t alignedMemU = memU;
    while ((alignedMemU & size) != 0) {
      alignedMemU++;
    }

    if ((alignedMemU + size) > (memU + paddedSize)) {
      // Unexpected error while aligning memory
      std::abort();
    }

    auto alignedMem = reinterpret_cast<std::byte*>(alignedMemU);
    return std::make_pair(mem, alignedMem);
  }

  size_t CurrFreeAddress = 8; // NOLINT
  // Some sandboxed encode functions as regular pointers, others use an
  // indirection table. Using an indirection table here as this is the more
  // complicated case
  mutable std::mutex function_table_mutex;
  mutable std::vector<const void*> function_table;

public:
  using T_LongLongType = int64_t;
  using T_LongType = int32_t;
  using T_IntType = int32_t;
  using T_PointerType = uint32_t;
  using T_ShortType = int16_t;

  inline static const uint32_t SandboxMemorySize = 0xFFF;
  inline static const uintptr_t SandboxMemoryBaseMask =
    ~(static_cast<uintptr_t>(SandboxMemorySize));
  uintptr_t UnalignedSandboxMemory;
  uintptr_t SandboxMemoryBase;

protected:
  template<typename... T_Args>
  inline void impl_create_sandbox(T_Args...)
  {
    auto unalignedAndAligned = pow2SizeAlignedMalloc(SandboxMemorySize);
    UnalignedSandboxMemory =
      reinterpret_cast<uintptr_t>(unalignedAndAligned.first);
    SandboxMemoryBase = reinterpret_cast<uintptr_t>(unalignedAndAligned.second);
  }

  inline void impl_destroy_sandbox()
  {
    delete[](reinterpret_cast<std::byte*>(UnalignedSandboxMemory));
  }

  template<typename T>
  inline void* impl_get_unsandboxed_pointer(T_PointerType p) const
  {
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      std::lock_guard<std::mutex> lock(function_table_mutex);
      return const_cast<void*>(function_table[p]);
    } else {
      return reinterpret_cast<void*>(SandboxMemoryBase +
                                     static_cast<uintptr_t>(p));
    }
  }

  template<typename T>
  inline T_PointerType impl_get_sandboxed_pointer(const void* p) const
  {
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      std::lock_guard<std::mutex> lock(function_table_mutex);
      int len = function_table.size();
      function_table.push_back(p);
      return static_cast<T_PointerType>(len);
    } else {
      return static_cast<T_PointerType>(reinterpret_cast<uintptr_t>(p) -
                                        SandboxMemoryBase);
    }
  }

  template<typename T>
  static inline void* impl_get_unsandboxed_pointer_no_ctx(
    T_PointerType p,
    const void* example_unsandboxed_ptr,
    TestSandbox* (*expensive_sandbox_finder)(
      const void* example_unsandboxed_ptr))
  {
    RLBOX_DEBUG_ASSERT(example_unsandboxed_ptr != nullptr);
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      // swizzling function pointers needs access to the function pointer tables
      // and thus cannot be done without context
      RLBOX_UNUSED(example_unsandboxed_ptr);
      auto sandbox = expensive_sandbox_finder(example_unsandboxed_ptr);
      return sandbox->impl_get_unsandboxed_pointer<T>(p);
    } else {
      auto mask = SandboxMemoryBaseMask &
                  reinterpret_cast<uintptr_t>(example_unsandboxed_ptr);
      return reinterpret_cast<void*>(mask + static_cast<uintptr_t>(p));
    }
  }

  template<typename T>
  static inline T_PointerType impl_get_sandboxed_pointer_no_ctx(
    const void* p,
    const void* example_unsandboxed_ptr,
    TestSandbox* (*expensive_sandbox_finder)(
      const void* example_unsandboxed_ptr))
  {
    RLBOX_DEBUG_ASSERT(example_unsandboxed_ptr != nullptr);
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      // swizzling function pointers needs access to the function pointer tables
      // and thus cannot be done without context
      RLBOX_UNUSED(example_unsandboxed_ptr);
      auto sandbox = expensive_sandbox_finder(example_unsandboxed_ptr);
      return sandbox->impl_get_sandboxed_pointer<T>(p);
    } else {
      auto ret = SandboxMemorySize & reinterpret_cast<uintptr_t>(p);
      return static_cast<T_PointerType>(ret);
    }
  }

  inline T_PointerType impl_malloc_in_sandbox(size_t size)
  {
    auto ret = static_cast<T_PointerType>(CurrFreeAddress);
    // Malloc normally produces pointers aligned to sizeof(uintptr) bytes
    size_t roundedTo8Size = (size + 7) & ~7;
    CurrFreeAddress += roundedTo8Size;
    if (CurrFreeAddress > SandboxMemorySize) {
      std::abort();
    }
    return ret;
  }

  inline void impl_free_in_sandbox(T_PointerType) {}

  static inline bool impl_is_in_same_sandbox(const void* p1, const void* p2)
  {
    auto mask1 = SandboxMemoryBaseMask & reinterpret_cast<uintptr_t>(p1);
    auto mask2 = SandboxMemoryBaseMask & reinterpret_cast<uintptr_t>(p2);
    return mask1 == mask2;
  }

  inline bool impl_is_pointer_in_sandbox_memory(const void* p)
  {
    auto mask = SandboxMemoryBaseMask & reinterpret_cast<uintptr_t>(p);
    return mask == SandboxMemoryBase;
  }

  inline bool impl_is_pointer_in_app_memory(const void* p)
  {
    return !(impl_is_pointer_in_sandbox_memory(p));
  }

  inline size_t impl_get_total_memory() { return SandboxMemorySize; }

  inline void* impl_get_memory_location()
  {
    return reinterpret_cast<void*>(SandboxMemoryBase);
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
      "#define RLBOX_USE_STATIC_CALLS rlbox_test_sandbox_lookup_symbol\n\n"
      "to their code, to ensure that static calls are handled correctly.");

    return nullptr;
  }

#define rlbox_test_sandbox_lookup_symbol(func_name)                            \
  reinterpret_cast<void*>(&func_name) /* NOLINT */

  template<typename T_Ret, typename... T_Args>
  inline T_PointerType impl_register_callback(void*, void*)
  {
    return 0;
  }

  static inline std::pair<TestSandbox*, void*>
  impl_get_executed_callback_sandbox_and_key()
  {
    TestSandbox* s = nullptr;
    void* k = nullptr;
    return std::make_pair(s, k);
  }

  template<typename T_Ret, typename... T_Args>
  inline void impl_unregister_callback(void*)
  {}
};
}
