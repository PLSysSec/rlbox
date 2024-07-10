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
 * elements of the int array
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

/**
 * @brief External "app" version test_array_arg - a function that adds
 * elements of the u32 array
 */
// static
int test_array_arg_u32(const uint32_t* aArr, int aCount);

using TSbxInt = rlbox_sandbox_type_test_ptr::sbx_int;
using TSbxPtr = rlbox_sandbox_type_test_ptr::sbx_pointer;

/**
 * @brief Internal "sandbox" version of test_array_arg - a function that adds
 * elements of the int array. This function has to be written in the sandbox
 * ABI.
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

/**
 * @brief Internal "sandbox" version of test_array_arg_u32 - a function that
 * adds elements of the u32 array. This function has to be written in the
 * sandbox ABI.
 *
 * @param aSandboxMemory is the pointer to the sandboxes' internal heap
 * @param aArrIdx is the array ptr as an index into `aSandboxMemory`
 * @param aCount is the number of elements in the array
 * @return TSbxInt is the sum of the elements in the array
 */
static TSbxInt test_array_arg_u32_internal(char* aSandboxMemory,
                                           TSbxPtr aArrIdx, TSbxInt aCount) {
  TSbxInt sum = 0;
  char* arr = aSandboxMemory + aArrIdx;

  for (TSbxInt i = 0; i < aCount; i++) {
    char* val_ptr = arr + sizeof(uint32_t) * i;
    uint32_t copy = 0;
    memcpy(&copy, val_ptr, sizeof(uint32_t));
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
  tainted_test_ptr<int[3]> t_testarr = {5, 7, 12};

  REQUIRE(t_testarr[0].UNSAFE_unverified() == 5);
  REQUIRE(t_testarr[1].UNSAFE_unverified() == 7);
  REQUIRE(t_testarr[2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS(t_testarr[3].UNSAFE_unverified() == 0);

  tainted_test_ptr<int> zero = 0;
  REQUIRE(t_testarr[zero + 0].UNSAFE_unverified() == 5);
  REQUIRE(t_testarr[zero + 1].UNSAFE_unverified() == 7);
  REQUIRE(t_testarr[zero + 2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS(t_testarr[zero + 3].UNSAFE_unverified() == 0);

  {
    std::array<int, 3> expected = {5, 7, 12};
    auto result = t_testarr.UNSAFE_unverified(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  {
    std::array<TSbxInt, 3> expected = {5, 7, 12};
    auto result = t_testarr.UNSAFE_sandboxed(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg, t_testarr, 3);
  REQUIRE(ret.UNSAFE_unverified() == 24);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted volatile array of ints operates correctly",
          "[tainted arrays]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  tainted_test_ptr<int(*)[3]> t_testarr = sandbox.malloc_in_sandbox<int[3]>();

  (*t_testarr)[0] = 5;
  (*t_testarr)[1] = 7;
  (*t_testarr)[2] = 12;

  REQUIRE((*t_testarr)[0].UNSAFE_unverified() == 5);
  REQUIRE((*t_testarr)[1].UNSAFE_unverified() == 7);
  REQUIRE((*t_testarr)[2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS((*t_testarr)[3].UNSAFE_unverified() == 0);

  tainted_test_ptr<int> zero = 0;
  REQUIRE((*t_testarr)[zero + 0].UNSAFE_unverified() == 5);
  REQUIRE((*t_testarr)[zero + 1].UNSAFE_unverified() == 7);
  REQUIRE((*t_testarr)[zero + 2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS((*t_testarr)[zero + 3].UNSAFE_unverified() == 0);

  {
    std::array<int, 3> expected = {5, 7, 12};
    auto result = (*t_testarr).UNSAFE_unverified(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  {
    std::array<TSbxInt, 3> expected = {5, 7, 12};
    auto result = (*t_testarr).UNSAFE_sandboxed(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg, (*t_testarr), 3);
  REQUIRE(ret.UNSAFE_unverified() == 24);

  sandbox.free_in_sandbox(t_testarr);
  sandbox.destroy_sandbox();
}

TEST_CASE("tainted array of std ints operates correctly", "[tainted arrays]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  [[maybe_unused]] tainted_test_ptr<rlbox_uint32_t[3]> t_val;
  [[maybe_unused]] tainted_test_ptr<rlbox_uint32_t[3]> t_val_init = {0};
  tainted_test_ptr<rlbox_uint32_t[3]> t_testarr = {5, 7, 12};

  REQUIRE(t_testarr[0].UNSAFE_unverified() == 5);
  REQUIRE(t_testarr[1].UNSAFE_unverified() == 7);
  REQUIRE(t_testarr[2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS(t_testarr[3].UNSAFE_unverified() == 0);

  tainted_test_ptr<rlbox_uint32_t> zero = 0;
  REQUIRE(t_testarr[zero + 0].UNSAFE_unverified() == 5);
  REQUIRE(t_testarr[zero + 1].UNSAFE_unverified() == 7);
  REQUIRE(t_testarr[zero + 2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS(t_testarr[zero + 3].UNSAFE_unverified() == 0);

  {
    std::array<uint32_t, 3> expected = {5, 7, 12};
    auto result = t_testarr.UNSAFE_unverified(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  {
    std::array<uint32_t, 3> expected = {5, 7, 12};
    auto result = t_testarr.UNSAFE_sandboxed(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  tainted_test_ptr<int> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg_u32, t_testarr, 3);
  REQUIRE(ret.UNSAFE_unverified() == 24);

  sandbox.destroy_sandbox();
}

TEST_CASE("tainted volatile array of std ints operates correctly",
          "[tainted arrays]") {
  rlbox_sandbox_test_ptr sandbox;
  sandbox.create_sandbox();

  tainted_test_ptr<rlbox_uint32_t(*)[3]> t_testarr =
      sandbox.malloc_in_sandbox<rlbox_uint32_t[3]>();

  (*t_testarr)[0] = 5;
  (*t_testarr)[1] = 7;
  (*t_testarr)[2] = 12;

  REQUIRE((*t_testarr)[0].UNSAFE_unverified() == 5);
  REQUIRE((*t_testarr)[1].UNSAFE_unverified() == 7);
  REQUIRE((*t_testarr)[2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS((*t_testarr)[3].UNSAFE_unverified() == 0);

  tainted_test_ptr<rlbox_uint32_t> zero = 0;
  REQUIRE((*t_testarr)[zero + 0].UNSAFE_unverified() == 5);
  REQUIRE((*t_testarr)[zero + 1].UNSAFE_unverified() == 7);
  REQUIRE((*t_testarr)[zero + 2].UNSAFE_unverified() == 12);
  REQUIRE_THROWS((*t_testarr)[zero + 3].UNSAFE_unverified() == 0);

  {
    std::array<uint32_t, 3> expected = {5, 7, 12};
    auto result = (*t_testarr).UNSAFE_unverified(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  {
    std::array<uint32_t, 3> expected = {5, 7, 12};
    auto result = (*t_testarr).UNSAFE_sandboxed(sandbox);
    REQUIRE(std::is_same_v<decltype(expected), decltype(result)>);
    REQUIRE(memcmp(&(expected[0]), &(result[0]), sizeof(expected)) == 0);
  }

  tainted_test_ptr<rlbox_uint32_t> ret =
      test_ptr_sandbox_invoke(sandbox, test_array_arg_u32, (*t_testarr), 3);
  REQUIRE(ret.UNSAFE_unverified() == 24);

  sandbox.free_in_sandbox(t_testarr);
  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
