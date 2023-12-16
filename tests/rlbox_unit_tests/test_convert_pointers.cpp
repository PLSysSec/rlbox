/**
 * @file test_convert_pointers.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that pointer conversion works as expected.
 */

#include <memory>

#include "test_include.hpp"

#include "rlbox_wrapper_traits.hpp"

class sandbox_same_pointer_rep
    : public rlbox::rlbox_sandbox_plugin_base<sandbox_same_pointer_rep> {
 public:
  template <typename T>
  using tainted = tainted_relocatable<T, sandbox_same_pointer_rep>;

  template <typename T>
  using tainted_volatile =
      tainted_volatile_standard<T, sandbox_same_pointer_rep>;

  using sbx_pointer = void*;

  template <typename T>
  inline void* impl_get_sandboxed_pointer(T aPtr) const {
    auto ret = reinterpret_cast<uintptr_t>(aPtr);
    return reinterpret_cast<void*>(ret);
  }

  template <typename T>
  inline T impl_get_unsandboxed_pointer(void* aPtr) const {
    auto ret = reinterpret_cast<uintptr_t>(aPtr);
    return reinterpret_cast<T>(ret);
  }

  template <typename T>
  inline bool impl_is_pointer_in_sandbox_memory(
      [[maybe_unused]] T aPtr) const noexcept {
    return true;
  }
};

TEST_CASE("Test pointer conversions with same abi", "[pointer conversion]") {
  rlbox::rlbox_sandbox<sandbox_same_pointer_rep> s;

  auto ptr_own = std::make_unique<char>();
  char* ptr = ptr_own.get();
  void* null = nullptr;

  auto* ptr_sbx = s.get_sandboxed_pointer(ptr);
  REQUIRE(ptr == ptr_sbx);
  REQUIRE(ptr == s.get_unsandboxed_pointer<decltype(ptr)>(ptr_sbx));

  auto* null_sbx = s.get_sandboxed_pointer(null);
  REQUIRE(null == null_sbx);
  REQUIRE(null == s.get_unsandboxed_pointer<decltype(null)>(null_sbx));
}

class sandbox_different_pointer_rep
    : public rlbox::rlbox_sandbox_plugin_base<sandbox_different_pointer_rep> {
 public:
  template <typename T>
  using tainted = tainted_relocatable<T, sandbox_different_pointer_rep>;

  template <typename T>
  using tainted_volatile =
      tainted_volatile_standard<T, sandbox_different_pointer_rep>;

  using sbx_pointer = void*;

  template <typename T>
  inline void* impl_get_sandboxed_pointer(T aPtr) const {
    uintptr_t ret = reinterpret_cast<uintptr_t>(aPtr) - 1;
    return reinterpret_cast<void*>(ret);
  }

  template <typename T>
  inline T impl_get_unsandboxed_pointer(void* aPtr) const {
    uintptr_t ret = reinterpret_cast<uintptr_t>(aPtr) + 1;
    return reinterpret_cast<T>(ret);
  }

  template <typename T>
  inline bool impl_is_pointer_in_sandbox_memory(
      [[maybe_unused]] T aPtr) const noexcept {
    return true;
  }
};

TEST_CASE("Test pointer conversions with different abi",
          "[pointer conversion]") {
  rlbox::rlbox_sandbox<sandbox_different_pointer_rep> s;

  auto ptr_own = std::make_unique<char>();
  char* ptr = ptr_own.get();
  void* null = nullptr;

  auto* ptr_sbx = s.get_sandboxed_pointer(ptr);
  void* expected = ptr - 1;
  REQUIRE(expected == ptr_sbx);
  REQUIRE(ptr == s.get_unsandboxed_pointer<decltype(ptr)>(ptr_sbx));

  auto* null_sbx = s.get_sandboxed_pointer(null);
  REQUIRE(null == null_sbx);
  REQUIRE(null == s.get_unsandboxed_pointer<decltype(null)>(null_sbx));
}
