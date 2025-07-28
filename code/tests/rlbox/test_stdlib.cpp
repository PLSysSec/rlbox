#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

#include "test_include.hpp"

using rlbox::memset;
using rlbox::tainted;

// NOLINTNEXTLINE
TEST_CASE("test sandbox_reinterpret_cast", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const auto testVal = 0xAB;
  auto ptr = sandbox.malloc_in_sandbox<uint64_t>();
  *ptr = testVal;
  auto ptr2 = rlbox::sandbox_reinterpret_cast<uint32_t*>(ptr);

  REQUIRE(std::is_same_v<decltype(ptr), tainted<uint64_t*, TestSandbox>>);
  REQUIRE(std::is_same_v<decltype(ptr2), tainted<uint32_t*, TestSandbox>>);
  REQUIRE(ptr2->UNSAFE_unverified() == testVal);

  sandbox.free_in_sandbox(ptr);
  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test sandbox_const_cast", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const auto testVal = 0xAB;
  auto ptr = sandbox.malloc_in_sandbox<const uint64_t>();
  auto ptr2 = rlbox::sandbox_const_cast<uint64_t*>(ptr);
  *ptr2 = testVal;

  REQUIRE(std::is_same_v<decltype(ptr), tainted<const uint64_t*, TestSandbox>>);
  REQUIRE(std::is_same_v<decltype(ptr2), tainted<uint64_t*, TestSandbox>>);
  REQUIRE(ptr->UNSAFE_unverified() == testVal);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test sandbox_static_cast", "[stdlib]")
{
  const uint64_t a = std::numeric_limits<uint64_t>::max();
  const auto b = static_cast<uint32_t>(a);
  tainted<uint64_t, TestSandbox> t_a = a;
  auto t_b = rlbox::sandbox_static_cast<uint32_t>(t_a);
  REQUIRE(std::is_same_v<decltype(t_b), tainted<uint32_t, TestSandbox>>);
  REQUIRE(b == t_b.UNSAFE_unverified());
}

// NOLINTNEXTLINE
TEST_CASE("test memset", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  auto initVal = sandbox.malloc_in_sandbox<unsigned int>(12); // NOLINT
  auto fifth = initVal + 4;

  const uint32_t max32Val = 0xFFFFFFFF;

  // Memset with untainted val and untainted size
  for (int i = 0; i < 12; i++) { // NOLINT
    *(initVal + i) = max32Val;
  }
  memset(sandbox, fifth, 0, sizeof(tainted<unsigned int, TestSandbox>) * 4);

  for (int i = 0; i < 4; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }
  for (int i = 4; i < 8; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == 0);
  }
  for (int i = 8; i < 12; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }

  // Memset with tainted val and untainted size
  tainted<int, TestSandbox> val = 0;
  for (int i = 0; i < 12; i++) { // NOLINT
    *(initVal + i) = max32Val;
  }
  memset(sandbox, fifth, val, sizeof(tainted<unsigned int, TestSandbox>) * 4);

  for (int i = 0; i < 4; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }
  for (int i = 4; i < 8; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == 0);
  }
  for (int i = 8; i < 12; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }

  // Memset with tainted val and untainted size
  tainted<size_t, TestSandbox> size =
    sizeof(tainted<unsigned int, TestSandbox>) * 4;
  for (int i = 0; i < 12; i++) { // NOLINT
    *(initVal + i) = max32Val;
  }
  memset(sandbox, fifth, val, size);

  for (int i = 0; i < 4; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }
  for (int i = 4; i < 8; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == 0);
  }
  for (int i = 8; i < 12; i++) { // NOLINT
    REQUIRE(*((initVal + i).UNSAFE_unverified()) == max32Val);
  }

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test memcpy", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const uint32_t max32Val = 0xFFFFFFFF;

  /////////////// Check with a tainted source

  // Alloc and zero initialize 12 int dest buffer
  auto dest = sandbox.malloc_in_sandbox<unsigned int>(12); // NOLINT
  for (int i = 0; i < 12; i++) { // NOLINT
    *(dest + i) = 0;
  }

  // Alloc and max initialize 12 int src buffer
  auto src = sandbox.malloc_in_sandbox<unsigned int>(12); // NOLINT
  for (int i = 0; i < 12; i++) { // NOLINT
    *(src + i) = max32Val;
  }

  auto dest_fifth = dest + 4;
  auto src_fifth = src + 4;

  memcpy(sandbox,
         dest_fifth,
         src_fifth,
         sizeof(tainted<unsigned int, TestSandbox>) * 4);

  // Check that destination looks as expected
  for (int i = 0; i < 4; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == 0);
  }
  for (int i = 4; i < 8; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == max32Val);
  }
  for (int i = 8; i < 12; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == 0);
  }

  /////////////// Check with a untainted source

  // zero initialize 12 int dest buffer
  for (int i = 0; i < 12; i++) { // NOLINT
    *(dest + i) = 0;
  }

  // Alloc and max initialize 12 int untainted src buffer
  unsigned int* src2 = new unsigned int[12]; // NOLINT
  for (int i = 0; i < 12; i++) {             // NOLINT
    *(src2 + i) = max32Val;                  // NOLINT
  }

  auto src2_fifth = src2 + 4;                // NOLINT
  memcpy(sandbox,
         dest_fifth,
         src2_fifth,
         sizeof(tainted<unsigned int, TestSandbox>) * 4);

  // Check that destination looks as expected
  for (int i = 0; i < 4; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == 0);
  }
  for (int i = 4; i < 8; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == max32Val);
  }
  for (int i = 8; i < 12; i++) { // NOLINT
    REQUIRE(*((dest + i).UNSAFE_unverified()) == 0);
  }

  delete[] src2; // NOLINT
  sandbox.free_in_sandbox(src);
  sandbox.free_in_sandbox(dest);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test strncpy", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const uint32_t max32Val = 0xFFFFFFFF;

  /////////////// Check with a tainted source

  // Alloc and zero initialize dest string
  auto dest = sandbox.malloc_in_sandbox<char>(12); // NOLINT
  for (int i = 0; i < 12; i++) { // NOLINT
    *(dest + i) = 0;
  }

  // Alloc and max initialize src string
  auto src = sandbox.malloc_in_sandbox<char>(12); // NOLINT
  memcpy(src.unverified_safe_pointer_because(12, "Known size"), "Hello", 6);

  strncpy(sandbox,
         dest,
         src,
         12);

  // Check that destination looks as expected
  REQUIRE(
    strncmp(
      dest.unverified_safe_pointer_because(12, "Known size"),
      src.unverified_safe_pointer_because(12, "Known size"),
      6
    ) == 0
  );

  /////////////// Check with an untainted source

  // zero initialize dest string
  for (int i = 0; i < 12; i++) { // NOLINT
    *(dest + i) = 0;
  }

  // Alloc and max initialize src string
  auto src2 = new char[12]; // NOLINT
  memcpy(src2, "Hello", 6);

  strncpy(sandbox,
         dest,
         src,
         12);

  // Check that destination looks as expected
  REQUIRE(
    strncmp(
      dest.unverified_safe_pointer_because(12, "Known size"),
      src2,
      6
    ) == 0
  );

  delete[] src2; // NOLINT
  sandbox.free_in_sandbox(src);
  sandbox.free_in_sandbox(dest);

  sandbox.destroy_sandbox();
}

static int normalize(int a)
{
  if (a > 0) {
    return 1;
  } else if (a < 0) {
    return -1;
  }
  return 0;
}

// NOLINTNEXTLINE
TEST_CASE("test memcmp", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  const char* buffer1 = "abcd";
  const char* buffer2 = "abCD";

  const uint32_t max_length = 100;
  auto buffer1_t = sandbox.malloc_in_sandbox<char>(max_length); // NOLINT
  auto buffer2_t = sandbox.malloc_in_sandbox<char>(max_length); // NOLINT

  std::strncpy(buffer1_t.UNSAFE_unverified(), buffer1, max_length);
  std::strncpy(buffer2_t.UNSAFE_unverified(), buffer2, max_length);

  auto b1b1 = std::memcmp(buffer1, buffer1, strlen(buffer1));
  auto b1b2 = std::memcmp(buffer1, buffer2, strlen(buffer1));
  auto b2b1 = std::memcmp(buffer2, buffer1, strlen(buffer1));

  printf("MEMCMP Debugging line 1\n");
  // NOLINTNEXTLINE
  auto tb1b1 = rlbox::memcmp(sandbox, buffer1_t, buffer1, strlen(buffer1))
                 .unverified_safe_because("test");
  printf("MEMCMP Debugging line 2\n");
  // NOLINTNEXTLINE
  auto tb1b2 = rlbox::memcmp(sandbox, buffer1_t, buffer2, strlen(buffer1))
                 .unverified_safe_because("test");
  printf("MEMCMP Debugging line 3\n");
  // NOLINTNEXTLINE
  auto tb2b1 = rlbox::memcmp(sandbox, buffer2_t, buffer1, strlen(buffer1))
                 .unverified_safe_because("test");

  printf("MEMCMP Debugging line 4\n");
  // NOLINTNEXTLINE
  auto tb1tb1 = rlbox::memcmp(sandbox, buffer1_t, buffer1_t, strlen(buffer1))
                  .unverified_safe_because("test");
  printf("MEMCMP Debugging line 5\n");
  // NOLINTNEXTLINE
  auto tb1tb2 = rlbox::memcmp(sandbox, buffer1_t, buffer2_t, strlen(buffer1))
                  .unverified_safe_because("test");
  printf("MEMCMP Debugging line 6\n");
  // NOLINTNEXTLINE
  auto tb2tb1 = rlbox::memcmp(sandbox, buffer2_t, buffer1_t, strlen(buffer1))
                  .unverified_safe_because("test");
  printf("MEMCMP Debugging line 7\n");

  b1b1 = normalize(b1b1);
  b1b2 = normalize(b1b2);
  b2b1 = normalize(b2b1);
  tb1b1 = normalize(tb1b1);
  tb1b2 = normalize(tb1b2);
  tb2b1 = normalize(tb2b1);
  tb1tb1 = normalize(tb1tb1);
  tb1tb2 = normalize(tb1tb2);
  tb2tb1 = normalize(tb2tb1);

  REQUIRE(b1b1 == tb1b1);
  REQUIRE(b1b2 == tb1b2);
  REQUIRE(b2b1 == tb2b1);

  REQUIRE(b1b1 == tb1tb1);
  REQUIRE(b1b2 == tb1tb2);
  REQUIRE(b2b1 == tb2tb1);

  sandbox.free_in_sandbox(buffer1_t);
  sandbox.free_in_sandbox(buffer2_t);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test grant deny access single", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  char* src = static_cast<char*>(malloc(sizeof(char))); // NOLINT
  const char test_val = 42;
  *src = test_val;

  bool used_copy = false;

  auto transfered =
    rlbox::copy_memory_or_grant_access(sandbox, src, 1, true, used_copy);
  REQUIRE((*transfered == test_val).unverified_safe_because("test"));

  auto transfered2 =
    rlbox::copy_memory_or_deny_access(sandbox, transfered, 1, true, used_copy);
  REQUIRE(*transfered2 == test_val);

  free(transfered2);

  sandbox.destroy_sandbox();
}

// NOLINTNEXTLINE
TEST_CASE("test grant deny access many", "[stdlib]")
{
  rlbox::rlbox_sandbox<TestSandbox> sandbox;
  sandbox.create_sandbox();

  float* src = static_cast<float*>(malloc(2 * sizeof(float))); // NOLINT
  const float test_val1 = 42;
  const float test_val2 = 43;
  src[0] = test_val1;
  src[1] = test_val2;

  bool used_copy = false;

  auto transfered =
    rlbox::copy_memory_or_grant_access(sandbox, src, 2, true, used_copy);
  REQUIRE((transfered[0] == test_val1).unverified_safe_because("test"));
  REQUIRE((transfered[1] == test_val2).unverified_safe_because("test"));

  auto transfered2 =
    rlbox::copy_memory_or_deny_access(sandbox, transfered, 2, true, used_copy);
  REQUIRE(transfered2[0] == test_val1);
  REQUIRE(transfered2[1] == test_val2);

  free(transfered2);

  sandbox.destroy_sandbox();
}
