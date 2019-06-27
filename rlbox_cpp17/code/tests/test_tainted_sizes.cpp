#include "test_include.hpp"

using CallbackType = int (*)(uint32_t, const char*, std::array<uint32_t, 1>);
using CallbackType2 =
  int (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

struct test
{
  uint64_t fieldLong;
  const char* fieldString;
  uint32_t fieldBool;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std::array<char, 8> fieldFixedArr; // NOLINT(readability-magic-numbers)
  int (*fieldFnPtr)(uint32_t, const char*, std::array<uint32_t, 1>);
  struct unknownClass* fieldUnknownPtr;
  void* voidPtr;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std::array<CallbackType, 4> fnArray; // NOLINT(readability-magic-numbers)
};

using rlbox::tainted;
using rlbox::tainted_volatile;

// NOLINTNEXTLINE
TEST_CASE("Tainted sizes work as expected", "[tainted_size]")
{
  using TSbx = void*;
  REQUIRE(sizeof(tainted<long long, TSbx>) == sizeof(long long)); // NOLINT
  REQUIRE(sizeof(tainted<long, TSbx>) == sizeof(long));           // NOLINT
  REQUIRE(sizeof(tainted<int, TSbx>) == sizeof(int));             // NOLINT
  REQUIRE(sizeof(tainted<void*, TSbx>) == sizeof(void*));         // NOLINT
  REQUIRE(sizeof(tainted<test, TSbx>) == sizeof(test));           // NOLINT
  REQUIRE(sizeof(tainted<test*, TSbx>) == sizeof(test*));         // NOLINT

  REQUIRE(sizeof(tainted_volatile<long long, TSbx>) ==
          sizeof(long long));                                      // NOLINT
  REQUIRE(sizeof(tainted_volatile<long, TSbx>) == sizeof(long));   // NOLINT
  REQUIRE(sizeof(tainted_volatile<int, TSbx>) == sizeof(int));     // NOLINT
  REQUIRE(sizeof(tainted_volatile<void*, TSbx>) == sizeof(void*)); // NOLINT
  REQUIRE(sizeof(tainted_volatile<test, TSbx>) == sizeof(test));   // NOLINT
  REQUIRE(sizeof(tainted_volatile<test*, TSbx>) == sizeof(test*)); // NOLINT
}