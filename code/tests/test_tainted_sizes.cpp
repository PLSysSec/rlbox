#include <cstdint>

#include "test_include.hpp"
#include "test_tainted_structs.hpp"

using rlbox::tainted;
using rlbox::tainted_volatile;

// NOLINTNEXTLINE
TEST_CASE("RLBox trait types do not introduce vtables", "[tainted_trait]")
{
  REQUIRE(sizeof(rlbox::sandbox_wrapper_base) < sizeof(uintptr_t)); // NOLINT
  REQUIRE(sizeof(rlbox::sandbox_wrapper_base_of<int>) <
          sizeof(uintptr_t));                                         // NOLINT
  REQUIRE(sizeof(rlbox::tainted_base<int, int>) < sizeof(uintptr_t)); // NOLINT
  REQUIRE(sizeof(rlbox::tainted_base_impl<tainted, int, int>) <
          sizeof(uintptr_t)); // NOLINT
}

template<typename T>
using T_Convert =
  rlbox::detail::convert_to_sandbox_equivalent_t<T, TestSandbox>;

// NOLINTNEXTLINE
TEST_CASE("Tainted sizes work as expected", "[tainted_size]")
{

  REQUIRE(sizeof(tainted<long long, TestSandbox>) ==
          sizeof(long long));                                    // NOLINT
  REQUIRE(sizeof(tainted<long, TestSandbox>) == sizeof(long));   // NOLINT
  REQUIRE(sizeof(tainted<int, TestSandbox>) == sizeof(int));     // NOLINT
  REQUIRE(sizeof(tainted<void*, TestSandbox>) == sizeof(void*)); // NOLINT
  REQUIRE(sizeof(tainted<testStruct, TestSandbox>) ==
          sizeof(testStruct)); // NOLINT
  REQUIRE(sizeof(tainted<testStruct*, TestSandbox>) ==
          sizeof(testStruct*)); // NOLINT

  REQUIRE(sizeof(tainted_volatile<long long, TestSandbox>) ==
          sizeof(TestSandbox::T_LongLongType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<long, TestSandbox>) ==
          sizeof(TestSandbox::T_LongType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<int, TestSandbox>) ==
          sizeof(TestSandbox::T_IntType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<void*, TestSandbox>) ==
          sizeof(TestSandbox::T_PointerType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<testStruct, TestSandbox>) ==
          sizeof(T_Convert<testStruct>)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<testStruct*, TestSandbox>) ==
          sizeof(T_Convert<testStruct*>)); // NOLINT
}