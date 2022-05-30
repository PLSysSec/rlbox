/**
 * @file test_malloc.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that allocations in the sandbox work as expected
 */

#include <type_traits>

#include "test_include.hpp"

#include "rlbox.hpp"

struct test_struct {
  int a;
  long b;
};

TEST_CASE("test allocation operates correctly", "[allocation]") {
  rlbox_sandbox_test sandbox;
  sandbox.create_sandbox();

  {
    tainted_test<int*> a = sandbox.malloc_in_sandbox<int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<int**> a = sandbox.malloc_in_sandbox<int*>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<int*>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_pointer));
  }
  {
    tainted_test<const int*> a = sandbox.malloc_in_sandbox<const int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<const int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<volatile int*> a = sandbox.malloc_in_sandbox<volatile int>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<volatile int>() ==
            sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    tainted_test<std::add_pointer_t<int[3]>> a =
        sandbox.malloc_in_sandbox<int[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<int[3]>() ==
            3 * sizeof(typename rlbox_sandbox_type_test::sbx_int));
  }
  {
    using int_ptr = int*;
    tainted_test<std::add_pointer_t<int_ptr[3]>> a =
        sandbox.malloc_in_sandbox<int_ptr[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<int_ptr[3]>() ==
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
    REQUIRE(sandbox.get_object_size_for_malloc<test_struct>() >=
            2 * padded_field_size);
  }
  {
    tainted_test<rlbox::rlbox_uint64_t*> a =
        sandbox.malloc_in_sandbox<rlbox::rlbox_uint64_t>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<rlbox::rlbox_uint64_t>() ==
            sizeof(uint64_t));
  }
  {
    tainted_test<std::add_pointer_t<rlbox::rlbox_uint64_t[3]>> a =
        sandbox.malloc_in_sandbox<rlbox::rlbox_uint64_t[3]>();
    sandbox.free_in_sandbox(a);
    REQUIRE(sandbox.get_object_size_for_malloc<rlbox::rlbox_uint64_t[3]>() ==
            3 * sizeof(uint64_t));
  }

  sandbox.destroy_sandbox();
}

namespace rlbox {
/**
 * @brief Sandbox we will use for testing class allocations with a larger ABIs
 */
class rlbox_largerabi_sandbox
    : public rlbox_sandbox_plugin_base<rlbox_largerabi_sandbox> {
 public:
  using sbx_short = int32_t;

  template <typename T>
  using tainted = tainted_fixed_aligned<T, rlbox_test_sandbox>;

  inline rlbox_status_code impl_create_sandbox() {
    return rlbox_status_code::SUCCESS;
  }

  inline rlbox_status_code impl_destroy_sandbox() {
    return rlbox_status_code::SUCCESS;
  }

  template <typename T>
  inline sbx_pointer impl_malloc_in_sandbox(size_t aCount) {
    return malloc(aCount);
  }

  template <typename T>
  inline void impl_free_in_sandbox([[maybe_unused]] sbx_pointer aPtr) {
    free(aPtr);
  }

  inline bool impl_is_pointer_in_sandbox_memory(const void* /* aPtr */) const {
    return true;
  }

  template <typename T>
  [[nodiscard]] inline sbx_pointer impl_get_sandboxed_pointer(T aPtr) const {
    // NOLINTNEXTLINE(google-readability-casting)
    return (sbx_pointer)(aPtr);
  }

  template <typename T>
  [[nodiscard]] inline T impl_get_unsandboxed_pointer(sbx_pointer aPtr) const {
    return reinterpret_cast<T>(aPtr);
  }
};
}  // namespace rlbox

RLBOX_DEFINE_BASE_TYPES_FOR(testlarger, rlbox_largerabi_sandbox);

// Below test is commented out for now until we add support for larger ABIs and
// `rlbox_lib_load_classes`

// TEST_CASE("test class allocation for larger ABI fails without definition",
//           "[allocation]") {
//   rlbox_sandbox_testlarger sandbox;
//   sandbox.create_sandbox();

//   REQUIRE_THROWS(sandbox.malloc_in_sandbox<test_struct>());

//   sandbox.destroy_sandbox();
// }