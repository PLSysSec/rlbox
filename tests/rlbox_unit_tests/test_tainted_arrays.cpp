/**
 * @file test_tainted_arrays.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that tainted wrappers handle arrays correctly
 */

// #include "test_include.hpp"

// static int test_array_arg(int* aArr, int aCount) {
//   int sum = 0;
//   for (int i = 0; i <= aCount; i++) {
//     sum += aArr[i];
//   }
//   return sum;
// }

// TEST_CASE("tainted array of ints operates correctly", "[tainted arrays]") {
//   rlbox_sandbox_test sandbox;
//   sandbox.create_sandbox();

//   //   int val[3] = {5, 7, 12};
//   //   const int expected = test_array_arg(val, 3);
//   //   tainted_test<int[3]> t_val = val;
//   tainted_test<int*> t_val = nullptr;
//   tainted_test<int> ret =
//       test_sandbox_invoke(sandbox, test_array_arg, t_val, 3);
//   REQUIRE(ret.UNSAFE_unverified() == 12);

//   sandbox.destroy_sandbox();
// }