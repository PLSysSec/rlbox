/**
 * @file test_include.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Include file to be used by tests. Contains some common headers and
 * functions used by tests
 */

#pragma once

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"

#include "rlbox.hpp"
// IWYU pragma: end_exports

#include "rlbox_function_traits.hpp"

#include <iostream>

using namespace rlbox;

/**
 * @brief Helper type that prints stringified version of the types given to it
 *
 * @tparam T is the set of types to print
 */
template <typename... T>
void rlbox_test_helper_print_type() {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  std::cout << __FUNCSIG__ << std::endl;
#else
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

/**
 * @brief A struct that is used to represent an allocation that is aligned to
 * its size. Eg: a 1k allocation will be aligned to 1k, a 512 byte allocation
 * will be aligned to 512 bytes and so on. The struct assumes we get the aligned
 * allocation, by overallocating (or padding) a larger buffer.
 */
struct aligned_alloc_t {
  /**
   * @brief An allocation which may be padded. The larger allocation means that
   * a pointer in the middle of this allocation can be considered an aligned
   * allocation.
   */
  char* full_allocation;
  /**
   * @brief The size of the padded allocation
   */
  size_t full_size;
  /**
   * @brief The pointer to memory which is aligned and has at least as much size
   * as requested. This field is a pointer into the @ref full_allocation field.
   */
  char* mem;
  /**
   * @brief The requested size of the aligned allocation
   */
  size_t size;
};

/**
 * @brief A function that returns a buffer of size `aSize`, aligned to `aSize`
 * @param aSize is the size of the buffer being allocated.
 * @return aligned_alloc_t is the allocation details, containing both
 * information of the aligned allocation and the padding necessary for the over
 * allocation.
 */
inline aligned_alloc_t rlbox_aligned_malloc(size_t aSize) {
  size_t paddedSize = aSize * 2 + 1;
  char* mem = new char[paddedSize];
  uintptr_t memU = reinterpret_cast<uintptr_t>(mem);
  if ((memU & aSize) == 0) {
    aligned_alloc_t ret{mem, paddedSize, mem, aSize};
    return ret;
  }

  uintptr_t alignedMemU = memU;
  while ((alignedMemU & aSize) != 0) {
    alignedMemU++;
  }

  if ((alignedMemU + aSize) > (memU + paddedSize)) {
    // Unexpected error while aligning memory
    std::abort();
  }

  auto alignedMem = reinterpret_cast<char*>(alignedMemU);

  aligned_alloc_t ret{mem, paddedSize, alignedMem, aSize};
  return ret;
}

/**
 * @brief Free the aligned allocation
 * @param alloc is the allocation we are freeing
 */
inline void rlbox_aligned_free(aligned_alloc_t alloc) {
  delete[] alloc.full_allocation;
}

namespace rlbox {
/**
 * @brief Common sandbox functionality that we will reuse in other sandboxes
 */
template <typename TSbx>
class rlbox_noop_arena_sandbox_base : public rlbox_sandbox_plugin_base<TSbx> {
 private:
  const size_t sandbox_mem_size = size_t(4) * 1024;
  aligned_alloc_t sandbox_memory_alloc{nullptr, 0, nullptr, 0};
  size_t bump_index = 16;

 public:
  char* sandbox_memory = nullptr;

  using sbx_short = int8_t;
  using sbx_int = int16_t;
  using sbx_long = int16_t;
  using sbx_longlong = int32_t;
  using sbx_pointer = uint32_t;

  template <typename T>
  using tainted = tainted_fixed_aligned<T, TSbx>;

  template <typename T>
  using tainted_volatile = tainted_volatile_standard<T, TSbx>;

  inline rlbox_status_code impl_create_sandbox() {
    sandbox_memory_alloc = rlbox_aligned_malloc(sandbox_mem_size);
    sandbox_memory = sandbox_memory_alloc.mem;
    return rlbox_status_code::SUCCESS;
  }

  inline rlbox_status_code impl_destroy_sandbox() {
    rlbox_aligned_free(sandbox_memory_alloc);
    return rlbox_status_code::SUCCESS;
  }

  template <typename TFunc, typename... TArgs>
  inline auto impl_invoke_with_func_ptr(void* aFuncPtr, TArgs&&... aArgs) {
    auto cast_func_ptr = reinterpret_cast<TFunc*>(aFuncPtr);
    return (*cast_func_ptr)(aArgs...);
  }

  template <typename T>
  inline sbx_pointer impl_malloc_in_sandbox(size_t aCount) {
    sbx_pointer ret = bump_index;
    bump_index += aCount;

    detail::dynamic_check(bump_index < sandbox_mem_size,
                          "Ran out of sandbox memory");

    auto roundTo8 = [](size_t numToRound) -> size_t {
      size_t multiple = 8;
      return ((numToRound + multiple - 1) / multiple) * multiple;
    };

    bump_index = roundTo8(bump_index);

    return ret;
  }

  template <typename T>
  inline void impl_free_in_sandbox([[maybe_unused]] sbx_pointer aPtr) {}

  inline bool impl_is_pointer_in_sandbox_memory(const void* aPtr) const {
    bool ret =
        aPtr >= sandbox_memory && aPtr < (sandbox_memory + sandbox_mem_size);
    return ret;
  }

  template <typename T>
  [[nodiscard]] inline sbx_pointer impl_get_sandboxed_pointer(T aPtr) const {
    auto ret = reinterpret_cast<uintptr_t>(aPtr) -
               reinterpret_cast<uintptr_t>(sandbox_memory);
    // NOLINTNEXTLINE(google-readability-casting)
    return (sbx_pointer)(ret);
  }

  template <typename T>
  [[nodiscard]] inline T impl_get_unsandboxed_pointer(sbx_pointer aPtr) const {
    char* ret = sandbox_memory + aPtr;
    return reinterpret_cast<T>(ret);
  }
};

/**
 * @brief Sandbox we will use for rlbox testing for primtive values
 */
class rlbox_noop_ptr_arena_sandbox
    : public rlbox_noop_arena_sandbox_base<rlbox_noop_ptr_arena_sandbox> {
 public:
  template <typename TFunc, typename... TArgs>
  inline auto impl_invoke_with_func_ptr(void* aFuncPtr, TArgs&&... aArgs) {
    using TFuncPrepend = detail::prepend_func_arg_t<TFunc, char*>;
    auto cast_func_ptr = reinterpret_cast<TFuncPrepend*>(aFuncPtr);
    return (*cast_func_ptr)(sandbox_memory, aArgs...);
  }
};

#define noop_ptr_arena_sandbox_invoke(sandbox, func_name, ...)            \
  sandbox_invoke_internal(sandbox, decltype(func_name), func_name,        \
                          reinterpret_cast<void*>(&func_name##_internal), \
                          ##__VA_ARGS__)
/**
 * @brief Sandbox we will use for rlbox testing for pointer values. This values
 * passes the arena memory to the function as the first argument.
 */
class rlbox_noop_arena_sandbox
    : public rlbox_noop_arena_sandbox_base<rlbox_noop_arena_sandbox> {};

#define noop_arena_sandbox_invoke(sandbox, func_name, ...)         \
  sandbox_invoke_internal(sandbox, decltype(func_name), func_name, \
                          reinterpret_cast<void*>(&func_name), ##__VA_ARGS__)

/**
 * @brief Sandbox that has a larger abi than the host, that we will use for
 * rlbox testing
 */
class rlbox_noop_arena_largerabi_sandbox
    : public rlbox_noop_arena_sandbox_base<rlbox_noop_arena_largerabi_sandbox> {
 public:
  using sbx_short = int32_t;
};

#define noop_arena_largerabi_sandbox_invoke noop_arena_sandbox_invoke

/**
 * @brief Sandbox that has a smaller abi than the host, that we will use for
 * rlbox testing
 */
class rlbox_noop_arena_smallerabi_sandbox
    : public rlbox_noop_arena_sandbox_base<
          rlbox_noop_arena_smallerabi_sandbox> {
 public:
  using sbx_short = int8_t;
};

}  // namespace rlbox

#define noop_arena_smallerabi_sandbox_invoke noop_arena_sandbox_invoke

// Above is plugin code. Below we have the application code using different
// plugins

RLBOX_DEFINE_BASE_TYPES_FOR(test, rlbox_noop_arena_sandbox);
#define test_sandbox_invoke noop_arena_sandbox_invoke

RLBOX_DEFINE_BASE_TYPES_FOR(test_ptr, rlbox_noop_ptr_arena_sandbox);
#define test_ptr_sandbox_invoke noop_ptr_arena_sandbox_invoke

RLBOX_DEFINE_BASE_TYPES_FOR(test_largerabi, rlbox_noop_arena_largerabi_sandbox);
#define test_largerabi_sandbox_invoke noop_arena_largerabi_sandbox_invoke

RLBOX_DEFINE_BASE_TYPES_FOR(test_smallerabi,
                            rlbox_noop_arena_smallerabi_sandbox);
#define test_smallerabi_sandbox_invoke noop_arena_smallerabi_sandbox_invoke
