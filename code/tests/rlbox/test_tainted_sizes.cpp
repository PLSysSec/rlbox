#include "test_include.hpp"
#include "test_tainted_structs.hpp"

using rlbox::tainted;
using rlbox::tainted_volatile;

template<typename T>
using T_Convert =
  rlbox::detail::convert_to_sandbox_equivalent_t<T, TestSandbox>;

// NOLINTNEXTLINE
TEST_CASE("Tainted sizes work as expected", "[tainted_size]")
{
  tainted<testVarietyStruct, TestSandbox> a{};
  UNUSED(a);
  REQUIRE(sizeof(tainted<long long, TestSandbox>) ==
          sizeof(long long));                                    // NOLINT
  REQUIRE(sizeof(tainted<long, TestSandbox>) == sizeof(long));   // NOLINT
  REQUIRE(sizeof(tainted<int, TestSandbox>) == sizeof(int));     // NOLINT
  REQUIRE(sizeof(tainted<void*, TestSandbox>) == sizeof(void*)); // NOLINT
  REQUIRE(sizeof(tainted<testVarietyStruct, TestSandbox>) ==
          sizeof(testVarietyStruct)); // NOLINT
  REQUIRE(sizeof(tainted<testVarietyStruct*, TestSandbox>) ==
          sizeof(testVarietyStruct*)); // NOLINT

  REQUIRE(sizeof(tainted_volatile<long long, TestSandbox>) ==
          sizeof(TestSandbox::T_LongLongType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<long, TestSandbox>) ==
          sizeof(TestSandbox::T_LongType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<int, TestSandbox>) ==
          sizeof(TestSandbox::T_IntType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<short, TestSandbox>) ==
          sizeof(TestSandbox::T_ShortType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<void*, TestSandbox>) ==
          sizeof(TestSandbox::T_PointerType)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<testVarietyStruct, TestSandbox>) ==
          sizeof(T_Convert<testVarietyStruct>)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<testVarietyStruct*, TestSandbox>) ==
          sizeof(T_Convert<testVarietyStruct*>)); // NOLINT
}