#include <type_traits>

#include "test_include.hpp"
#include "test_tainted_structs.hpp" // IWYU pragma: keep

using rlbox::sandbox_callback;
using rlbox::tainted;
using rlbox::tainted_volatile;
using rlbox::detail::rlbox_is_tainted_v;
using rlbox::detail::rlbox_remove_wrapper_t;

// NOLINTNEXTLINE
TEST_CASE("rlbox_is_tainted_v", "[wrapper_traits]")
{
  // Unwrapped
  REQUIRE(!rlbox_is_tainted_v<int>);
  REQUIRE(!rlbox_is_tainted_v<int*>);
  REQUIRE(!rlbox_is_tainted_v<char[4]>); // NOLINT
  REQUIRE(!rlbox_is_tainted_v<testVarietyStruct>);

  // fundamental tainted
  REQUIRE(rlbox_is_tainted_v<tainted<int, TestSandbox>>);
  REQUIRE(!rlbox_is_tainted_v<tainted_volatile<int, TestSandbox>>);

  // enum tainted
  REQUIRE(rlbox_is_tainted_v<tainted<testBasicEnum, TestSandbox>>);
  REQUIRE(!rlbox_is_tainted_v<tainted_volatile<testBasicEnum, TestSandbox>>);

  // pointer tainted
  REQUIRE(rlbox_is_tainted_v<tainted<int*, TestSandbox>>);
  REQUIRE(!rlbox_is_tainted_v<tainted_volatile<int*, TestSandbox>>);

  // static array tainted
  // NOLINTNEXTLINE
  REQUIRE(rlbox_is_tainted_v<tainted<char[4], TestSandbox>>); // NOLINT
  REQUIRE(!rlbox_is_tainted_v<tainted_volatile<char[4],       // NOLINT
                                               TestSandbox>>);

  // struct tainted
  REQUIRE(rlbox_is_tainted_v<tainted<testVarietyStruct, TestSandbox>>);
  REQUIRE(
    !rlbox_is_tainted_v<tainted_volatile<testVarietyStruct, TestSandbox>>);

  // sandbox_callback
  REQUIRE(!rlbox_is_tainted_v<sandbox_callback<int (*)(int), TestSandbox>>);
}

// NOLINTNEXTLINE
TEST_CASE("rlbox_remove_wrapper_t", "[wrapper_traits]")
{
  // Unwrapped
  REQUIRE(std::is_same_v<rlbox_remove_wrapper_t<int>, int>);
  REQUIRE(std::is_same_v<rlbox_remove_wrapper_t<int*>, int*>);
  REQUIRE(std::is_same_v<rlbox_remove_wrapper_t<char[4]>, char[4]>); // NOLINT
  REQUIRE(std::is_same_v<rlbox_remove_wrapper_t<testVarietyStruct>,
                         testVarietyStruct>);

  // fundamental tainted
  REQUIRE(
    std::is_same_v<rlbox_remove_wrapper_t<tainted<int, TestSandbox>>, int>);
  REQUIRE(
    std::is_same_v<rlbox_remove_wrapper_t<tainted_volatile<int, TestSandbox>>,
                   int>);

  // enum tainted
  REQUIRE(
    std::is_same_v<rlbox_remove_wrapper_t<tainted<testBasicEnum, TestSandbox>>,
                   testBasicEnum>);
  REQUIRE(std::is_same_v<
          rlbox_remove_wrapper_t<tainted_volatile<testBasicEnum, TestSandbox>>,
          testBasicEnum>);

  // pointer tainted
  REQUIRE(
    std::is_same_v<rlbox_remove_wrapper_t<tainted<int*, TestSandbox>>, int*>);
  REQUIRE(
    std::is_same_v<rlbox_remove_wrapper_t<tainted_volatile<int*, TestSandbox>>,
                   int*>);

  // static array tainted
  // NOLINTNEXTLINE
  REQUIRE(std::is_same_v<rlbox_remove_wrapper_t<tainted<char[4], TestSandbox>>,
                         char[4]>); // NOLINT
  REQUIRE(
    std::is_same_v<
      rlbox_remove_wrapper_t<tainted_volatile<char[4], TestSandbox>>, // NOLINT
      char[4]>);                                                      // NOLINT

  // struct tainted
  REQUIRE(std::is_same_v<
          rlbox_remove_wrapper_t<tainted<testVarietyStruct, TestSandbox>>,
          testVarietyStruct>);
  REQUIRE(
    std::is_same_v<
      rlbox_remove_wrapper_t<tainted_volatile<testVarietyStruct, TestSandbox>>,
      testVarietyStruct>);

  // sandbox_callback
  REQUIRE(std::is_same_v<
          rlbox_remove_wrapper_t<sandbox_callback<int (*)(int), TestSandbox>>,
          int (*)(int)>);
}