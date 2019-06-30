#include <cstdint>
#include <type_traits>

#include "test_include.hpp"

using rlbox::tainted;
using rlbox::tainted_volatile;

// NOLINTNEXTLINE
TEST_CASE("tainted assignment operates correctly", "[tainted_assignment]")
{
  const int RandomVal1 = 4;
  const int RandomVal2 = 5;

  // Avoid warnings about uninitialized var
  tainted<int, T_Sbx> a; // NOLINT
  a = RandomVal1;
  tainted<int, T_Sbx> b = RandomVal2;
  tainted<int, T_Sbx> c = b;
  tainted<int, T_Sbx> d; // NOLINT
  d = b;
  REQUIRE(a.UNSAFE_Unverified() == RandomVal1); // NOLINT
  REQUIRE(b.UNSAFE_Unverified() == RandomVal2); // NOLINT
  REQUIRE(c.UNSAFE_Unverified() == RandomVal2); // NOLINT
  REQUIRE(d.UNSAFE_Unverified() == RandomVal2); // NOLINT
}

// NOLINTNEXTLINE
TEST_CASE("tainted tainted_volatile conversion operates correctly",
          "[tainted_assignment]")
{
  T_Sbx sandbox;
  sandbox.create_sandbox();

  auto ptr = sandbox.malloc_in_sandbox<uint32_t>();
  REQUIRE(std::is_same_v<decltype(ptr), tainted<uint32_t*, T_Sbx>>);
  REQUIRE(ptr.UNSAFE_Unverified() != nullptr);

  auto& val = *ptr;
  REQUIRE(std::is_same_v<decltype(val), tainted_volatile<uint32_t, T_Sbx>&>);
  REQUIRE(std::is_same_v<decltype(tainted(val)), tainted<uint32_t, T_Sbx>>);

  REQUIRE(std::is_same_v<decltype(tainted(&val)), tainted<uint32_t*, T_Sbx>>);
  REQUIRE(std::is_same_v<decltype(tainted(&*ptr)), tainted<uint32_t*, T_Sbx>>);

  tainted<uint32_t**, T_Sbx> ptr2 = sandbox.malloc_in_sandbox<uint32_t*>();
  auto& deref = *ptr2;
  REQUIRE(std::is_same_v<decltype(deref), tainted_volatile<uint32_t*, T_Sbx>&>);
  REQUIRE(std::is_same_v<decltype(*deref), tainted_volatile<uint32_t, T_Sbx>&>);

  REQUIRE(std::is_same_v<decltype(**ptr2), tainted_volatile<uint32_t, T_Sbx>&>);

  sandbox.destroy_sandbox();
}
