#include <cstring>

#include "test_include.hpp"

using rlbox::sandbox_reinterpret_cast;
using rlbox::tainted;
using rlbox::tainted_opaque;

#include "test_tainted_structs.hpp"

// NOLINTNEXTLINE
TEST_CASE("tainted opaque operates correctly", "[tainted_opaque]")
{
  const int test_val = 5;
  tainted<int, TestSandbox> a = test_val;
  tainted_opaque<int, TestSandbox> b = a.to_opaque();
  auto c = rlbox::from_opaque(b);
  REQUIRE(c.UNSAFE_unverified() == test_val);

  b.set_zero();
  REQUIRE(rlbox::from_opaque(b).UNSAFE_unverified() == 0);

  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const auto fieldLong = 7;
  const auto strSize = 10;
  auto fieldString = sandbox.malloc_in_sandbox<char>(strSize);
  std::strncpy(fieldString.UNSAFE_unverified(), "Hello", strSize);
  const auto fieldBool = 1;

  tainted<testVarietyStruct, TestSandbox> s{};
  s.fieldLong = fieldLong;
  s.fieldString = sandbox_reinterpret_cast<const char*>(fieldString);
  s.fieldBool = fieldBool;
  //   char* temp = (&(s.fieldFixedArr[0]))->UNSAFE_unverified();
  //   std::strncpy(temp, "Bye", sizeof(s.fieldFixedArr));
  s.voidPtr = nullptr;

  tainted_opaque<testVarietyStruct, TestSandbox> s2 = s.to_opaque();
  auto s3 = rlbox::from_opaque(s2);

  REQUIRE(s3.fieldLong.UNSAFE_unverified() == fieldLong);
  REQUIRE(std::strcmp(s3.fieldString.UNSAFE_unverified(), "Hello") == 0);
  REQUIRE(s3.fieldBool.UNSAFE_unverified() == fieldBool);
  //   auto fixedArr = s3.fieldFixedArr.UNSAFE_unverified();
  //   REQUIRE(std::strcmp(&fixedArr[0], "Bye") == 0);

  tainted<void*, TestSandbox> voidPtr = s3.voidPtr;
  REQUIRE(voidPtr == nullptr);
  REQUIRE(s3.voidPtr.UNSAFE_unverified() == nullptr);
  REQUIRE(voidPtr.UNSAFE_unverified() == nullptr);

  tainted<const char*, TestSandbox> stringPtr = s3.fieldString;
  tainted_opaque<const char*, TestSandbox> stringOpaquePtr =
    stringPtr.to_opaque();
  stringOpaquePtr.set_zero();
  REQUIRE(rlbox::from_opaque(stringOpaquePtr).UNSAFE_unverified() == nullptr);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("tainted opaque free operates correctly", "[tainted_opaque]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();
  auto fieldString = sandbox.malloc_in_sandbox<char>(1).to_opaque();
  sandbox.free_in_sandbox(fieldString);
  sandbox.destroy_sandbox();
}