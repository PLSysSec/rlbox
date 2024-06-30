/**
 * @file test_tainted_arrays.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that tainted wrappers handle arrays correctly
 */

#include "test_include.hpp"

#include <string.h>

// NOLINTBEGIN(misc-const-correctness)

/**
 * @brief External "app" version test_array_arg - a function that adds
 * elements of the array
 */
// static
int test_array_arg(const int* aArr, int aCount);
// {
//   int sum = 0;

//   for (int i = 0; i < aCount; i++) {
//     sum += aArr[i];
//   }
//   return sum;
// }

using TSbxInt = rlbox_sandbox_type_test_ptr::sbx_int;
using TSbxPtr = rlbox_sandbox_type_test_ptr::sbx_pointer;

/**
 * @brief Internal "sandbox" version of test_array_arg - a function that adds
 * elements of the array Function that adds elements of the array. This function
 * has to be written in the sandbox ABI.
 *
 * @param aSandboxMemory is the pointer to the sandboxes' internal heap
 * @param aArrIdx is the array ptr as an index into `aSandboxMemory`
 * @param aCount is the number of elements in the array
 * @return TSbxInt is the sum of the elements in the array
 */
static TSbxInt test_array_arg_internal(char* aSandboxMemory, TSbxPtr aArrIdx,
                                       TSbxInt aCount) {
  TSbxInt sum = 0;
  char* arr = aSandboxMemory + aArrIdx;

  for (TSbxInt i = 0; i < aCount; i++) {
    char* val_ptr = arr + sizeof(TSbxInt) * i;
    TSbxInt copy = 0;
    memcpy(&copy, val_ptr, sizeof(TSbxInt));
    sum = static_cast<TSbxInt>(sum + copy);
  }
  return sum;
}

TEST_CASE("sandbox_invoke operates correctly with pointer arrays",
          "[tainted arrays]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  tainted_test_ptr<int*> t_val = sandbox.malloc_in_sandbox<int>();
  *t_val = 3;

  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg, t_val, 1);
  REQUIRE(ret.UNSAFE_unverified() == 3);

  sandbox.free_in_sandbox(t_val);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted array of ints operates correctly", "[tainted arrays]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  [[maybe_unused]] tainted_test_ptr<int[3]> t_val;
  [[maybe_unused]] tainted_test_ptr<int[3]> t_val_init = {0};
  [[maybe_unused]] tainted_test_ptr<int[3]> t_val_init2 = {1, 2, 3};

  REQUIRE(t_val_init2[0].UNSAFE_unverified() == 1);
  REQUIRE(t_val_init2[1].UNSAFE_unverified() == 2);
  REQUIRE(t_val_init2[2].UNSAFE_unverified() == 3);
  REQUIRE_THROWS(t_val_init2[3].UNSAFE_unverified() == 0);

  tainted_test_ptr<int> zero = 0;
  REQUIRE(t_val_init2[zero + 0].UNSAFE_unverified() == 1);
  REQUIRE(t_val_init2[zero + 1].UNSAFE_unverified() == 2);
  REQUIRE(t_val_init2[zero + 2].UNSAFE_unverified() == 3);
  REQUIRE_THROWS(t_val_init2[zero + 3].UNSAFE_unverified() == 0);

  [[maybe_unused]] tainted_test_ptr<int(*)[3]> t_val_ptr;
  tainted_test_ptr<int(*)[3]> t_vol_ptr_val =
      sandbox.malloc_in_sandbox<int[3]>();
  sandbox.free_in_sandbox(t_vol_ptr_val);

  tainted_test_ptr<int[3]> t_arr = {5, 7, 12};
  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg, t_arr, 3);
  REQUIRE(ret.UNSAFE_unverified() == 24);

  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
