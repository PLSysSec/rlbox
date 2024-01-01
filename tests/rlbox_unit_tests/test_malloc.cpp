/**
 * @file test_malloc.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that allocations in the sandbox work as expected
 */

#include <algorithm>
#include <cstddef>
#include <stdint.h>
#include <type_traits>

#include "test_include.hpp"

#include "rlbox.hpp"
#include "rlbox_stdint_types.hpp"

struct test_struct {
  int a;
  long b;
};

// NOLINTBEGIN(misc-const-correctness)

TEST_CASE("test allocation operates correctly", "[allocation]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  {
    tainted_test<int*> a = sandbox.malloc_in_sandbox<int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<int**> a = sandbox.malloc_in_sandbox<int*>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<int*>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_pointer));
  }
  {
    tainted_test<const int*> a = sandbox.malloc_in_sandbox<const int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<const int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<volatile int*> a = sandbox.malloc_in_sandbox<volatile int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<volatile int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<std::add_pointer_t<int[3]>> a =
        sandbox.malloc_in_sandbox<int[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<int[3]>() ==
            3 * sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    using int_ptr = int*;
    tainted_test<std::add_pointer_t<int_ptr[3]>> a =
        sandbox.malloc_in_sandbox<int_ptr[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<int_ptr[3]>() ==
            3 * sizeof(typename rlbox_sandbox_type_test::sbx_pointer));
  }
  {
    tainted_test<test_struct*> a = sandbox.malloc_in_sandbox<test_struct>();
    sandbox.free_in_sandbox(a);
    // We want to test the allocation size decided by RLBox.
    // For this we need to estimate the size of the struct test_struct in the
    // sandbox ABI. At a minimum, the size of the struct is size of the two
    // fields (int, long) Additionally, C pads the fields according to the size
    const size_t padded_field_size =
        std::max(sizeof(typename rlbox_sandbox_type_test::sbx_int),
                 sizeof(typename rlbox_sandbox_type_test::sbx_long));
    REQUIRE(sandbox.get_object_size_upperbound<test_struct>() >=
            2 * padded_field_size);
  }
  {
    tainted_test<rlbox::rlbox_uint32_t*> a =
        sandbox.malloc_in_sandbox<rlbox::rlbox_uint32_t>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<rlbox::rlbox_uint32_t>() ==
            sizeof(uint32_t));
  }
  {
    tainted_test<rlbox::rlbox_uint64_t*> a =
        sandbox.malloc_in_sandbox<rlbox::rlbox_uint64_t>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<rlbox::rlbox_uint64_t>() ==
            sizeof(uint64_t));
  }
  {
    tainted_test<std::add_pointer_t<rlbox::rlbox_uint64_t[3]>> a =
        sandbox.malloc_in_sandbox<rlbox::rlbox_uint64_t[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_upperbound<rlbox::rlbox_uint64_t[3]>() ==
            3 * sizeof(uint64_t));
  }

  sandbox.destroy_sandbox();
}

TEST_CASE("test class allocation for larger ABI fails without definition",
          "[allocation]") {
  rlbox_sandbox_test_largerabi sandbox;
  sandbox.create_sandbox();

  REQUIRE_THROWS(sandbox.malloc_in_sandbox<test_struct>());

  sandbox.destroy_sandbox();
}

// NOLINTEND(misc-const-correctness)
