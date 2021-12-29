#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

// IWYU pragma: no_forward_declare mpl_::na
#include "catch2/catch.hpp"
#include "libtest.h"
#include "rlbox.hpp"

#include "libtest_structs_for_cpp_api.h"
rlbox_load_structs_from_library(libtest); // NOLINT

using rlbox::app_pointer;
using rlbox::rlbox_sandbox;
using rlbox::tainted;
using namespace std::chrono;

#ifndef CreateSandbox
#  error "Define CreateSandbox before including this file"
#endif

#ifndef TestName
#  error "Define TestName before including this file"
#endif

#ifndef TestType
#  error "Define TestType before including this file"
#endif

#if _MSC_VER && !__INTEL_COMPILER
#  define NOINLINE __declspec(noinline)
#else
#  define NOINLINE __attribute__((noinline))
#endif

static tainted<int, TestType> exampleCallback(
  // NOLINTNEXTLINE(google-runtime-references)
  rlbox_sandbox<TestType>& sandbox,
  tainted<unsigned, TestType> a,
  tainted<const char*, TestType> b,
  // NOLINTNEXTLINE
  tainted<unsigned*, TestType> c)
{
  const unsigned upper_bound = 100;
  auto aCopy = a.copy_and_verify(
    [&](unsigned val) { return val > 0 && val < upper_bound ? val : -1U; });
  auto bCopy =
    b.copy_and_verify_string([&](std::unique_ptr<char[]> val) { // NOLINT
      auto raw = val.get();
      if (!raw) {
        return std::unique_ptr<char[]>(nullptr);
      }
      return std::strlen(raw) < upper_bound ? std::move(val) : nullptr;
    });

  auto cCopy = c.copy_and_verify_range(
    [&](std::unique_ptr<const unsigned[]> arr) { // NOLINT
      return arr[0] > 0 && arr[0] < upper_bound ? std::move(arr) : nullptr;
    },
    1);
  REQUIRE(cCopy[0] + 1 == aCopy); // NOLINT
  auto ret = aCopy + std::strlen(bCopy.get());

  // test reentrancy
  tainted<int*, TestType> pFoo = sandbox.template malloc_in_sandbox<int>();
  sandbox.free_in_sandbox(pFoo);
  return ret; // NOLINT
}

static tainted<int, TestType> exampleCallback2( // NOLINT(google-runtime-int)
  rlbox_sandbox<TestType>& /* sandbox */,
  tainted<unsigned long, TestType> val1, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val2, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val3, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val4, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val5, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val6) // NOLINT(google-runtime-int)
{
  return ((val1.UNSAFE_unverified() == 4)     // NOLINT
          && (val2.UNSAFE_unverified() == 5)  // NOLINT
          && (val3.UNSAFE_unverified() == 6)  // NOLINT
          && (val4.UNSAFE_unverified() == 7)  // NOLINT
          && (val5.UNSAFE_unverified() == 8)  // NOLINT
          && (val6.UNSAFE_unverified() == 9)) // NOLINT
           ? 11                               // NOLINT
           : -1;                              // NOLINT
}

static tainted<unsigned long, TestType>
exampleCallback3( // NOLINT(google-runtime-int)
  rlbox_sandbox<TestType>& /* sandbox */,
  tainted<unsigned long, TestType> val1, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val2) // NOLINT(google-runtime-int)
{
  return val1 + val2;
}

static tainted<float, TestType> cbFloat(rlbox_sandbox<TestType>& /* sandbox */,
                                        tainted<float, TestType> val)
{
  return val;
}

static tainted<double, TestType> cbDouble(
  rlbox_sandbox<TestType>& /* sandbox */,
  tainted<double, TestType> val)
{
  return val;
}

static tainted<long long int, TestType> cbLongLong(
  rlbox_sandbox<TestType>& /* sandbox */,
  tainted<long long int, TestType> val)
{
  return val;
}

NOINLINE
static unsigned long local_simpleAddNoPrintTest(unsigned long a,
                                                unsigned long b)
{
  return a + b;
}

NOINLINE
static unsigned long local_simpleCallbackLoop(unsigned long a,
                                              unsigned long b,
                                              unsigned long iterations,
                                              CallbackType3 callback)
{
  unsigned long ret = 0;
  for (unsigned long i = 0; i < iterations; i++) {
    ret += callback(a, b);
  }
  return ret;
}

// NOLINTNEXTLINE
TEST_CASE("sandbox glue tests " TestName, "[sandbox_glue_tests]")
{
  rlbox::rlbox_sandbox<TestType> sandbox;
  CreateSandbox(sandbox);

  const int upper_bound = 100;
#ifdef BENCHMARK_CUSTOM_ITERATIONS
  const int test_iterations = BENCHMARK_CUSTOM_ITERATIONS;
#else
  const int test_iterations = 1000000;
#endif

  tainted<char*, TestType> sb_string =
    sandbox.template malloc_in_sandbox<char>(upper_bound);
  // strcpy is safe here
  // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
  std::strcpy(sb_string.UNSAFE_unverified(), "Hello");

  SECTION("test simple function invocation") // NOLINT
  {
    const int val1 = 20;
    const int val2 = 22;
    tainted<int, TestType> a = val1;
    auto ret2 = sandbox.invoke_sandbox_function(simpleAddTest, a, val2);
    REQUIRE(ret2.UNSAFE_unverified() == (val1 + val2));
  }

  // Disabled until we have better support for std int types like std::unt64_t
  // SECTION("test 64 bit returns") // NOLINT
  // {
  //   const uint32_t val1 = 20;
  //   const auto u32Max = std::numeric_limits<std::uint32_t>::max();
  //   auto ret2 =
  //     sandbox.invoke_sandbox_function(simpleLongAddTest, u32Max, val1);
  //   auto result = static_cast<long>(u32Max) + val1; // NOLINT
  //   REQUIRE(ret2.UNSAFE_unverified() == result);
  // }

  SECTION("test function with stack params invocation") // NOLINT
  {
    const unsigned long val1 = 20;
    const unsigned long val2 = 23;
    const unsigned long val3 = 26;
    const unsigned long val4 = 29;
    const unsigned long val5 = 32;
    const unsigned long val6 = 35;
    const unsigned long val7 = 38;
    const unsigned long val8 = 41;
    const unsigned long val9 = 44;

    auto ret2 = sandbox.invoke_sandbox_function(stackParametersTest,
                                                val1,
                                                val2,
                                                val3,
                                                val4,
                                                val5,
                                                val6,
                                                val7,
                                                val8,
                                                val9);
    REQUIRE(ret2.UNSAFE_unverified() ==
            (val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9));
  }

  SECTION("test verification function") // NOLINT
  {
    const int val1 = 2;
    const int val2 = 3;
    auto result1 = sandbox.invoke_sandbox_function(simpleAddTest, val1, val2)
                     .copy_and_verify([&](int val) {
                       return val > 0 && val < upper_bound ? val : -1;
                     });
    REQUIRE(result1 == val1 + val2);
  }

  SECTION("test pointer verification function") // NOLINT
  {
    const int val1 = 4;

    tainted<int*, TestType> pa = sandbox.template malloc_in_sandbox<int>();
    *pa = val1;

    auto result1 = sandbox.invoke_sandbox_function(echoPointer, pa)
                     .copy_and_verify([&](std::unique_ptr<const int> val) {
                       return *val > 0 && *val < upper_bound ? *val : -1;
                     });
    REQUIRE(result1 == val1);

    auto result2 =
      sandbox.invoke_sandbox_function(echoPointer, pa)
        .copy_and_verify([&](std::unique_ptr<const int> val) {
          return *val > 0 && *val < upper_bound ? std::move(val) : nullptr;
        });

    REQUIRE(*result2 == val1);
    sandbox.free_in_sandbox(pa);
  }

  SECTION("test callback 1 and re-entrancy") // NOLINT
  {
    const unsigned cb_val_param = 4;

    auto cb_callback_param = sandbox.register_callback(exampleCallback);

    auto resultT = sandbox.invoke_sandbox_function(
      simpleCallbackTest, cb_val_param, sb_string, cb_callback_param);

    auto result = resultT.copy_and_verify(
      [&](int val) { return val > 0 && val < upper_bound ? val : -1; });
    REQUIRE(result == 10);
  }

  SECTION("test callback 2") // NOLINT
  {
    auto cb_callback_param = sandbox.register_callback(exampleCallback2);

    auto resultT = sandbox.invoke_sandbox_function(
      simpleCallbackTest2, 4, cb_callback_param);

    auto result = resultT.copy_and_verify([](int val) { return val; });
    REQUIRE(result == 11);
  }

  SECTION("test callback different returns") // NOLINT
  {
    {
      auto cb_callback_param = sandbox.register_callback(cbFloat);
      const float val = 1042.1;
      auto resultT = sandbox.invoke_sandbox_function(
        callbackTypeFloatTest, val, cb_callback_param);

      auto result = resultT.copy_and_verify([](float val) { return val; });
      REQUIRE(result == val);
    }
    {
      auto cb_callback_param = sandbox.register_callback(cbDouble);
      const double val = 1042.1;
      auto resultT = sandbox.invoke_sandbox_function(
        callbackTypeDoubleTest, val, cb_callback_param);

      auto result = resultT.copy_and_verify([](double val) { return val; });
      REQUIRE(result == val);
    }
    {
      auto cb_callback_param = sandbox.register_callback(cbLongLong);
      const long long val = -42;
      auto resultT = sandbox.invoke_sandbox_function(
        callbackTypeLongLongTest, val, cb_callback_param);

      auto result = resultT.copy_and_verify([](long long val) { return val; });
      REQUIRE(result == val);
    }
  }

  SECTION("test callback to an internal function") // NOLINT
  {
    auto fnPtr = sandbox.get_sandbox_function_address(internalCallback);

    tainted<testStruct*, TestType> pFoo =
      sandbox.template malloc_in_sandbox<testStruct>();
    pFoo->fieldFnPtr = fnPtr;

    auto resultT = sandbox.invoke_sandbox_function(
      simpleCallbackTest, static_cast<unsigned>(4), sb_string, fnPtr);

    auto result = resultT.copy_and_verify(
      [&](int val) { return val > 0 && val < upper_bound ? val : -1; });
    REQUIRE(result == 10);

    sandbox.free_in_sandbox(pFoo);
  }

  SECTION("test callback registration unregistration") // NOLINT
  {
    const uint32_t cb_iterations = 1024;
    for (uint32_t i = 0; i < cb_iterations; i++) {
      // NOLINTNEXTLINE
      rlbox::sandbox_callback<int (*)(unsigned, const char*, unsigned*),
                              TestType>
        cb_callback_param1;
      rlbox::sandbox_callback<int (*)(unsigned long,  // NOLINT
                                      unsigned long,  // NOLINT
                                      unsigned long,  // NOLINT
                                      unsigned long,  // NOLINT
                                      unsigned long,  // NOLINT
                                      unsigned long), // NOLINT
                              TestType>
        cb_callback_param2;

      cb_callback_param1 = sandbox.register_callback(exampleCallback);
      cb_callback_param2 = sandbox.register_callback(exampleCallback2);
      // destructor will unregister the cbs here
    }
  }

  SECTION("test echo and pointer locations") // NOLINT
  {
    const char* str = "Hello";

    // str is allocated in our heap, not the sandbox's heap
    REQUIRE(sandbox.is_pointer_in_app_memory(str));

    tainted<char*, TestType> temp =
      sandbox.template malloc_in_sandbox<char>(std::strlen(str) + 1);
    char* str_in_sbx = temp.UNSAFE_unverified();
    REQUIRE(sandbox.is_pointer_in_sandbox_memory(str_in_sbx));

    // strcpy is safe here
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
    std::strcpy(str_in_sbx, str);

    auto retStrRaw = sandbox.invoke_sandbox_function(simpleEchoTest, temp);
    // test some modifications of the string
    *retStrRaw = 'g';
    char* retStr = retStrRaw.copy_and_verify_string(
      [&](std::unique_ptr<char[]> val) { // NOLINT
        return std::strlen(val.get()) < upper_bound ? val.release() : nullptr;
      });

    REQUIRE(retStr != nullptr);
    REQUIRE(sandbox.is_pointer_in_app_memory(retStr));

    REQUIRE(std::strcmp(str, retStr) != 0);
    *retStrRaw = 'H';
    auto retStr2 = retStrRaw.copy_and_verify_string(
      [&](std::unique_ptr<char[]> val) { // NOLINT
        auto raw = val.get();
        if (!raw) {
          return std::unique_ptr<char[]>(nullptr);
        }
        return std::strlen(raw) < upper_bound ? std::move(val) : nullptr;
      });
    REQUIRE(std::strcmp(str, retStr2.get()) == 0);

    sandbox.free_in_sandbox(temp);
    delete[] retStr; // NOLINT
  }

  // Disable for now
  // SECTION("test integer conversions") // NOLINT
  // {
  //   const unsigned int zero = 0;
  //   const unsigned int one = 1;
  //   const unsigned int zero_with_upper = 0x81BFD800;
  //   const unsigned int one_with_upper = 0x81BFD801;

  //   auto ret1 = sandbox.invoke_sandbox_function(isNonNullChar, zero).UNSAFE_unverified();
  //   REQUIRE(ret1 == 0);

  //   auto ret2 = sandbox.invoke_sandbox_function(isNonNullChar, one).UNSAFE_unverified();
  //   REQUIRE(ret2 == 1);

  //   auto ret3 = sandbox.invoke_sandbox_function(isNonNullChar, zero_with_upper).UNSAFE_unverified();
  //   REQUIRE(ret3 == 0);

  //   auto ret4 = sandbox.invoke_sandbox_function(isNonNullChar, one_with_upper).UNSAFE_unverified();
  //   REQUIRE(ret4 == 1);
  // }

  SECTION("test floating point") // NOLINT
  {
    const float fVal1 = 1.0;
    const float fVal2 = 2.0;
    const double dVal1 = 1.0;
    const double dVal2 = 2.0;
    const auto defaultVal = -1.0;

    auto resultF =
      sandbox.invoke_sandbox_function(simpleFloatAddTest, fVal1, fVal2)
        .copy_and_verify([&](float val) {
          return val > 0 && val < upper_bound ? val : defaultVal;
        });
    REQUIRE(resultF == fVal1 + fVal2);

    auto resultD =
      sandbox.invoke_sandbox_function(simpleDoubleAddTest, dVal1, dVal2)
        .copy_and_verify([&](double val) {
          return val > 0 && val < upper_bound ? val : defaultVal;
        });
    REQUIRE(resultD == dVal1 + dVal2);

    // test float to double conversions
    auto resultFD =
      sandbox.invoke_sandbox_function(simpleFloatAddTest, dVal1, dVal2)
        .copy_and_verify([&](double val) {
          return val > 0 && val < upper_bound ? val : defaultVal;
        });
    REQUIRE(resultFD == dVal1 + dVal2);
  }

  SECTION("test pointer val add") // NOLINT
  {
    const double d1 = 1.0;
    const double d2 = 2.0;
    const auto defaultVal = -1.0;

    tainted<double*, TestType> p = sandbox.template malloc_in_sandbox<double>();
    *p = d1;

    auto resultD =
      sandbox.invoke_sandbox_function(simplePointerValAddTest, p, d2)
        .copy_and_verify([&](double val) {
          return val > 0 && val < upper_bound ? val : defaultVal;
        });
    REQUIRE(resultD == d1 + d2);
    sandbox.free_in_sandbox(p);
  }

  SECTION("test structure params") // NOLINT
  {
    tainted<testStruct, TestType> val{};
    val.fieldLong = 2;
    val.fieldString = rlbox::sandbox_const_cast<const char*>(sb_string);
    auto resultT = sandbox.invoke_sandbox_function(simpleTestStructParam, val);
    REQUIRE(resultT.UNSAFE_unverified() == 7);
  }

  SECTION("test structures") // NOLINT
  {
    auto resultT = sandbox.invoke_sandbox_function(simpleTestStructVal);
    auto result = resultT.copy_and_verify(
      [&](tainted<testStruct, TestType> val) -> testStruct {
        testStruct ret{};
        ret.fieldLong = val.fieldLong.UNSAFE_unverified();

        ret.fieldString = val.fieldString.copy_and_verify_string(
          [&](std::unique_ptr<const char[]> s_val) { // NOLINT
            return std::strlen(s_val.get()) < upper_bound ? s_val.release()
                                                          : nullptr;
          });

        ret.fieldBool = val.fieldBool.UNSAFE_unverified();

        auto fieldFixedArr = val.fieldFixedArr.UNSAFE_unverified();
        std::memcpy(
          &ret.fieldFixedArr[0], &fieldFixedArr, sizeof(fieldFixedArr));

        return ret;
      });
    REQUIRE(result.fieldLong == 7);
    REQUIRE(std::strcmp(result.fieldString, "Hello") == 0);
    REQUIRE(result.fieldBool == 1);
    REQUIRE(std::strcmp(&result.fieldFixedArr[0], "Bye") == 0);

    // writes should still go through
    resultT.fieldLong = 17;                               // NOLINT
    REQUIRE(resultT.fieldLong.UNSAFE_unverified() == 17); // NOLINT
    delete[] result.fieldString;
  }

  SECTION("test structure pointer") // NOLINT
  {
    auto resultT = sandbox.invoke_sandbox_function(simpleTestStructPtr);

    auto result = resultT.copy_and_verify(
      [&](std::unique_ptr<tainted<testStruct, TestType>> val) {
        testStruct ret{};
        ret.fieldLong = val->fieldLong.UNSAFE_unverified();

        ret.fieldString = val->fieldString.copy_and_verify_string(
          [&](std::unique_ptr<const char[]> s_val) { // NOLINT
            return std::strlen(s_val.get()) < upper_bound ? s_val.release()
                                                          : nullptr;
          });

        ret.fieldBool = val->fieldBool.UNSAFE_unverified();

        auto fieldFixedArr = val->fieldFixedArr.UNSAFE_unverified();
        std::memcpy(
          &ret.fieldFixedArr[0], &fieldFixedArr, sizeof(fieldFixedArr));

        return ret;
      });
    REQUIRE(result.fieldLong == 7);
    REQUIRE(std::strcmp(result.fieldString, "Hello") == 0);
    REQUIRE(result.fieldBool == 1);
    REQUIRE(std::strcmp(&result.fieldFixedArr[0], "Bye") == 0);

    // writes should still go through
    resultT->fieldLong = 17;                               // NOLINT
    REQUIRE(resultT->fieldLong.UNSAFE_unverified() == 17); // NOLINT

    // test & and * operators
    auto val3 =
      (*&resultT->fieldLong).copy_and_verify([](unsigned long val) { // NOLINT
        return val;
      });
    REQUIRE(val3 == 17); // NOLINT
    delete[] result.fieldString;
    sandbox.free_in_sandbox(resultT);
  }

  SECTION("test pointers in struct") // NOLINT
  {
    auto initVal = sandbox.template malloc_in_sandbox<char>();
    auto resultT =
      sandbox.invoke_sandbox_function(initializePointerStruct, initVal);
    auto result = resultT.copy_and_verify(
      [](tainted<pointersStruct, TestType> val) -> pointersStruct {
        pointersStruct ret{};
        ret.firstPointer = val.firstPointer.UNSAFE_unverified();
        ret.pointerArray[0] = val.pointerArray[0].UNSAFE_unverified();
        ret.pointerArray[1] = val.pointerArray[1].UNSAFE_unverified();
        ret.pointerArray[2] = val.pointerArray[2].UNSAFE_unverified();
        ret.pointerArray[3] = val.pointerArray[3].UNSAFE_unverified();
        ret.lastPointer = val.lastPointer.UNSAFE_unverified();
        return ret;
      });
    char* initValRaw = initVal.UNSAFE_unverified();
    sandbox.free_in_sandbox(initVal);

    REQUIRE(result.firstPointer == initValRaw);
    REQUIRE(result.pointerArray[0] == (initValRaw + 1)); // NOLINT
    REQUIRE(result.pointerArray[1] == (initValRaw + 2)); // NOLINT
    REQUIRE(result.pointerArray[2] == (initValRaw + 3)); // NOLINT
    REQUIRE(result.pointerArray[3] == (initValRaw + 4)); // NOLINT
    REQUIRE(result.lastPointer == (initValRaw + 5));     // NOLINT
  }

  SECTION("test 32-bit pointer edge cases") // NOLINT
  {
    auto initVal = sandbox.template malloc_in_sandbox<char>(upper_bound);
    auto initVal3 = initVal + 3; // NOLINT
    *initVal3 = 'v';

    auto resultT =
      sandbox.invoke_sandbox_function(initializePointerStructPtr, initVal);

    char* initValRaw = initVal.UNSAFE_unverified();
    // check that reading a pointer in an array doesn't read neighboring
    // elements
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[0].UNSAFE_unverified() == (initValRaw + 1));

    // check that a write doesn't overwrite neighboring elements
    resultT->pointerArray[0] = nullptr;
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[1].UNSAFE_unverified() == (initValRaw + 2));

    // check that array reference decay followed by a read doesn't read
    // neighboring elements
    tainted<char**, TestType> elRef = &(resultT->pointerArray[2]);
    REQUIRE((**elRef).UNSAFE_unverified() == 'v');

    // check that array reference decay followed by a write doesn't
    // overwrite neighboring elements
    *elRef = nullptr;
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[3].UNSAFE_unverified() == (initValRaw + 4));
    sandbox.free_in_sandbox(resultT);
    sandbox.free_in_sandbox(initVal);
  }

  SECTION("Function invocation measurements") // NOLINT
  {
    // Warm up the timer. The first call is always slow (at least on the test
    // platform)
    for (int i = 0; i < 10; i++) {
      auto val = high_resolution_clock::now();
      RLBOX_UNUSED(val);
    }

    const int val1 = 2;
    const int val2 = 3;

    // Baseline
    uint64_t result1 = 0;
    {
      auto enter_time = high_resolution_clock::now();
      for (int i = 0; i < test_iterations; i++) {
        // to make sure the optimizer doesn't try to be too clever and eliminate
        // the call
        result1 += local_simpleAddNoPrintTest(val1, val2);
      }
      auto exit_time = high_resolution_clock::now();

      int64_t ns = duration_cast<nanoseconds>(exit_time - enter_time).count();
      std::cout << "Unsandboxed function invocation time: "
                << (ns / test_iterations) << "\n";
    }

    // Sandbox
    uint64_t result2 = 0;
    {
      auto enter_time = high_resolution_clock::now();
      for (int i = 0; i < test_iterations; i++) {
        // to make sure the optimizer doesn't try to be too clever and eliminate
        // the call
        result2 +=
          sandbox.invoke_sandbox_function(simpleAddNoPrintTest, val1, val2)
            .unverified_safe_because("test");
      }
      auto exit_time = high_resolution_clock::now();

      int64_t ns = duration_cast<nanoseconds>(exit_time - enter_time).count();
      std::cout << "Sandboxed function invocation time: "
                << (ns / test_iterations) << "\n";
    }

    REQUIRE(result1 == result2);
  }

  SECTION("Callback invocation measurements") // NOLINT
  {
    auto cb_callback_param = sandbox.register_callback(exampleCallback3);

    const int val1 = 2;
    const int val2 = 3;

    // Baseline
    uint64_t result1;
    {
      auto enter_time = high_resolution_clock::now();
      result1 = local_simpleCallbackLoop(
        val1, val2, test_iterations, local_simpleAddNoPrintTest);
      auto exit_time = high_resolution_clock::now();
      int64_t ns = duration_cast<nanoseconds>(exit_time - enter_time).count();
      std::cout << "Unsandboxed callback invocation time: "
                << (ns / test_iterations) << "\n";
    }

    // Sandbox
    uint64_t result2;
    {
      auto enter_time = high_resolution_clock::now();
      result2 =
        sandbox
          .invoke_sandbox_function(
            simpleCallbackLoop, val1, val2, test_iterations, cb_callback_param)
          .unverified_safe_because("test");
      auto exit_time = high_resolution_clock::now();
      int64_t ns = duration_cast<nanoseconds>(exit_time - enter_time).count();
      std::cout << "Sandboxed callback invocation time: "
                << (ns / test_iterations) << "\n";
    }

    REQUIRE(result1 == result2);
  }

  SECTION("test grant deny access") // NOLINT
  {
    char* src =
      static_cast<char*>(malloc(sizeof(char))); // NOLINT
    const char test_val = 42;
    *src = test_val;

    bool used_copy;

    auto transfered = rlbox::copy_memory_or_grant_access(
      sandbox, src, 1, true, used_copy);
    REQUIRE((*transfered == test_val).unverified_safe_because("test"));

    auto transfered2 = rlbox::copy_memory_or_deny_access(
      sandbox, transfered, 1, true, used_copy);
    REQUIRE(*transfered2 == test_val);

    free(transfered2);
  }

  SECTION("app_ptr test")
  {
    void* ptr = malloc(sizeof(unsigned int));
    app_pointer<void*, TestType> app_ptr = sandbox.get_app_pointer(ptr);
    tainted<void*, TestType> app_ptr_tainted = app_ptr.to_tainted();

    void* original_ptr = sandbox.lookup_app_ptr(app_ptr_tainted);
    REQUIRE(ptr == original_ptr);
    free(ptr);
  }

  sandbox.free_in_sandbox(sb_string);

  sandbox.destroy_sandbox();
}