#include "test_include.hpp"

// NOLINTNEXTLINE
TEST_CASE("Type get_[un]sandboxed_pointer", "[get_sandboxed]")
{
  T_Sbx sandbox;
  const T_EmptySandbox_PointerType testPointerSboxRep1 = 0xCD;
  const T_EmptySandbox_PointerType testPointerSboxRep2 = 0xBC;
  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  void* testPointer1 = reinterpret_cast<void*>(
    static_cast<uintptr_t>(SandboxMemoryBase) + testPointerSboxRep1);
  // NOLINTNEXTLINE (cppcoreguidelines-pro-type-reinterpret-cast)
  void* testPointer2 = reinterpret_cast<void*>(
    static_cast<uintptr_t>(SandboxMemoryBase) + testPointerSboxRep2);
  const T_EmptySandbox_PointerType nullptrSboxRep = 0;

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
}