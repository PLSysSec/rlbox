/**
 * @file test_invoke_pointers.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that sandbox_invoke works as expected with tainted pointers.
 */

#include "test_include.hpp"

#include <string.h>

/**
 * @brief External "app" version test_array_arg - a function that creates an
 * array and sets each element to a value
 */
// static
int* create_array_with_val(int aCount, int aVal);
// {
//   int sum = 0;

//   for (int i = 0; i < aCount; i++) {
//     sum += aArr[i];
//   }
//   return sum;
// }

using TSbxInt = rlbox_sandbox_type_test_ptr::sbx_int;
using TSbxPtr = rlbox_sandbox_type_test_ptr::sbx_pointer;

rlbox_sandbox_test_ptr sandbox;

/**
 * @brief Internal "sandbox" version of create_array_with_val -  a function that
 * creates an array and sets each element to a value. This function has to be
 * written in the sandbox ABI.
 *
 * @param aSandboxMemory is the pointer to the sandboxes' internal heap
 * @param aCount is the number of elements of the array to be created
 * @param aVal is the value to set each element of the array
 * @return TSbxPtr is the array ptr as an index into `aSandboxMemory`
 */
static TSbxPtr create_array_with_val_internal(char* aSandboxMemory,
                                              TSbxInt aCount, TSbxInt aVal) {
  /// \todo Move the function call ABI to pass inner sandbox
  auto sandbox_inner = sandbox.get_inner_sandbox_impl();
  TSbxPtr arrIdx =
      sandbox_inner->impl_malloc_in_sandbox<TSbxInt>(sizeof(TSbxInt) * aCount);
  char* arr = aSandboxMemory + arrIdx;

  for (TSbxInt i = 0; i < aCount; i++) {
    char* val_ptr = arr + sizeof(TSbxInt) * i;
    memcpy(val_ptr, &aVal, sizeof(TSbxInt));
  }

  return arrIdx;
}

TEST_CASE("sandbox_invoke operates correctly when returning pointers",
          "[tainted pointers]") {
  sandbox.create_sandbox();
  const int count = 10;
  const int val = 5;

  // tainted_test_ptr<int*>
  // auto ptr =
  test_ptr_sandbox_invoke(sandbox, create_array_with_val, count, val);

  // rlbox_test_helper_print_type<decltype(ptr)>();

  // for(int i = 0; i < count; i++) {

  // }

  // sandbox.free_in_sandbox(ptr);

  sandbox.destroy_sandbox();
}
