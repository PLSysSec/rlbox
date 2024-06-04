/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers correctly convert.
 */

#include <type_traits>

#include "test_include.hpp"

#include "rlbox_types.hpp"

/**
 * @brief External "app" version test_bool_params - a function that checks
 * parameters passed in are both true or both false
 */
// static
int test_bool_params(int aArg1, int aArg2);

using TSbxInt = rlbox_sandbox_type_test_ptr::sbx_int;
using TSbxPtr = rlbox_sandbox_type_test_ptr::sbx_pointer;

/**
 * @brief Internal "sandbox" version of test_bool_params -a function that checks
 * parameters passed in are are both true or both false. This function has to be
 * written in the sandbox ABI.
 *
 * @param aSandboxMemory is the pointer to the sandboxes' internal heap
 * @param aArg1 is a boolean arg
 * @param aArg2 is a boolean arg
 * @return 1 if the arguments are equal, 0 otherwise
 */
static TSbxInt test_bool_params_internal([[maybe_unused]] char* aSandboxMemory,
                                         TSbxInt aArg1, TSbxInt aArg2) {
  if ((aArg1 == 0 && aArg2 == 0) || (aArg1 > 0 && aArg2 > 0)) {
    return 1;
  }
  return 0;
}

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted conversions]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int*> ptr = sandbox.malloc_in_sandbox<int>();
  REQUIRE(ptr.UNSAFE_unverified() != nullptr);

  ////////////

  // When things work correctly the next line is a pointer, which triggers
  // clang-tidy checks that suggest we declare it as "auto*". Supress this
  // check, as this a test, and we want to code to compile when the test may
  // fail.

  // NOLINTNEXTLINE (llvm-qualified-auto,readability-qualified-auto)
  [[maybe_unused]] auto addrof_taint_ptr = &ptr;
  REQUIRE(std::is_same_v<decltype(addrof_taint_ptr), tainted_test<int*>*>);

  auto& deref_taint_ptr = *ptr;
  REQUIRE(
      std::is_same_v<decltype(deref_taint_ptr), tainted_volatile_test<int>&>);

  ////////////

  REQUIRE(std::is_same_v<decltype(tainted_test<int>(deref_taint_ptr)),
                         tainted_test<int>>);

  ////////////

  [[maybe_unused]] auto addrof_deref_taint_ptr =
      static_cast<tainted_test<int*>>(&deref_taint_ptr);
  REQUIRE(std::is_same_v<decltype(addrof_deref_taint_ptr), tainted_test<int*>>);
  REQUIRE(
      std::is_same_v<decltype(tainted_test<int*>(&*ptr)), tainted_test<int*>>);

  ////////////

  tainted_test<int**> ptr2 = sandbox.malloc_in_sandbox<int*>();
  auto& deref = *ptr2;
  REQUIRE(std::is_same_v<decltype(deref), tainted_volatile_test<int*>&>);

  REQUIRE(std::is_same_v<decltype(*deref), tainted_volatile_test<int>&>);
  REQUIRE(std::is_same_v<decltype(**ptr2), tainted_volatile_test<int>&>);

  *ptr2 = ptr;
  **ptr2 = 3;

  REQUIRE(ptr->UNSAFE_unverified() == 3);

  ////////////

  sandbox.free_in_sandbox(ptr2);
  sandbox.free_in_sandbox(ptr);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted tainted_boolean_hint conversion operates correctly",
          "[tainted conversions]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  tainted_boolean_hint_test_ptr arg1 = true;
  tainted_test_ptr<bool> arg2 = arg1;

  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_bool_params, arg1, arg2);
  REQUIRE(ret.UNSAFE_unverified() == 1);

  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
