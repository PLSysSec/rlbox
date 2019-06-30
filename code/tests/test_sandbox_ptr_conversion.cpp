#include <cstdint>

#include "test_include.hpp"

using T_Ptr = typename TestSandbox::T_PointerType;

// NOLINTNEXTLINE
TEST_CASE("Type get_[un]sandboxed_pointer", "[get_sandboxed]")
{
  T_Sbx sandbox;
  sandbox.create_sandbox();

  const T_Ptr testPointerSboxRep1 = 0xCD;
  const T_Ptr testPointerSboxRep2 = 0xBC;
  uintptr_t base = sandbox.get_sandbox_impl()->SandboxMemoryBase;

  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  void* testPointer1 = reinterpret_cast<void*>(base + testPointerSboxRep1);
  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  void* testPointer2 = reinterpret_cast<void*>(base + testPointerSboxRep2);
  const T_Ptr nullptrSboxRep = 0;

  REQUIRE(T_Sbx::get_sandboxed_pointer<void>(testPointer1, testPointer2) ==
          testPointerSboxRep1); // NOLINT
  REQUIRE(T_Sbx::get_unsandboxed_pointer<void>(
            testPointerSboxRep1, testPointer2) == testPointer1); // NOLINT
  REQUIRE(sandbox.get_sandboxed_pointer<void>(testPointer1) ==
          testPointerSboxRep1); // NOLINT
  REQUIRE(sandbox.get_unsandboxed_pointer<void>(testPointerSboxRep1) ==
          testPointer1); // NOLINT

  REQUIRE(T_Sbx::get_sandboxed_pointer<void>(nullptr, testPointer2) ==
          nullptrSboxRep); // NOLINT
  REQUIRE(T_Sbx::get_unsandboxed_pointer<void>(nullptrSboxRep, testPointer2) ==
          nullptr); // NOLINT
  REQUIRE(sandbox.get_sandboxed_pointer<void>(nullptr) ==
          nullptrSboxRep); // NOLINT
  REQUIRE(sandbox.get_unsandboxed_pointer<void>(nullptrSboxRep) ==
          nullptr); // NOLINT

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("Type get_[un]sandboxed_pointer for const pointers",
          "[get_sandboxed]")
{
  T_Sbx sandbox;
  sandbox.create_sandbox();

  const T_Ptr testPointerSboxRep1 = 0xCD;
  const T_Ptr testPointerSboxRep2 = 0xBC;
  uintptr_t base = sandbox.get_sandbox_impl()->SandboxMemoryBase;

  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  auto testPointer1 = reinterpret_cast<const void*>(base + testPointerSboxRep1);
  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  auto testPointer2 = reinterpret_cast<const void*>(base + testPointerSboxRep2);

  REQUIRE(T_Sbx::get_sandboxed_pointer<const void>(
            testPointer1, testPointer2) == testPointerSboxRep1); // NOLINT
  REQUIRE(T_Sbx::get_unsandboxed_pointer<const void>(
            testPointerSboxRep1, testPointer2) == testPointer1); // NOLINT
  REQUIRE(sandbox.get_sandboxed_pointer<const void>(testPointer1) ==
          testPointerSboxRep1); // NOLINT
  REQUIRE(sandbox.get_unsandboxed_pointer<const void>(testPointerSboxRep1) ==
          testPointer1); // NOLINT

  sandbox.destroy_sandbox();
}