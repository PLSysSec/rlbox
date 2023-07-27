/**
 * @file test_tainted_assignment.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the various tainted wrappers correctly convert.
 */

#include <type_traits>

#include "test_include.hpp"

TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted tainted_volatile conversion]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  tainted_test<int*> ptr = sandbox.malloc_in_sandbox<int>();
  REQUIRE(std::is_base_of_v<tainted_interface_sbx<rlbox_sandbox_type_test>,
                            decltype(ptr)>);
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
      tainted_test<int*>(&deref_taint_ptr);
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
