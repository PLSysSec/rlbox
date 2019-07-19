#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

// IWYU pragma: no_forward_declare mpl_::na
#include "libtest.h"
#include "test_sandbox_glue.hpp"

using rlbox::RLBox_Verify_Status;
using rlbox::RLBoxSandbox;
using rlbox::tainted;

template<typename TestType>
static tainted<int, TestType> exampleCallback(
  // NOLINTNEXTLINE(google-runtime-references)
  RLBoxSandbox<TestType>& sandbox,
  tainted<unsigned, TestType> a,
  tainted<const char*, TestType> b,
  // NOLINTNEXTLINE
  tainted<unsigned*, TestType> c)
{
  const unsigned upper_bound = 100;
  auto aCopy = a.copy_and_verify(
    [](unsigned val) {
      return val > 0 && val < upper_bound ? RLBox_Verify_Status::SAFE
                                          : RLBox_Verify_Status::UNSAFE;
    },
    -1U);
  auto bCopy = b.copy_and_verify_string(
    [](const char* val) {
      return strlen(val) < upper_bound ? RLBox_Verify_Status::SAFE
                                       : RLBox_Verify_Status::UNSAFE;
    },
    nullptr);

  unsigned* def_ptr = nullptr;

  auto cCopy = c.copy_and_verify_range(
    [](const unsigned* arr) {
      return *arr > 0 && *arr < upper_bound ? RLBox_Verify_Status::SAFE
                                            : RLBox_Verify_Status::UNSAFE;
    },
    1,
    def_ptr);
  REQUIRE(cCopy[0] + 1 == aCopy); // NOLINT
  auto ret = aCopy + strlen(bCopy);
  delete[] bCopy; // NOLINT
  delete[] cCopy; // NOLINT

  // test reentrancy
  tainted<int*, TestType> pFoo = sandbox.template malloc_in_sandbox<int>();
  sandbox.free_in_sandbox(pFoo);
  return ret; // NOLINT
}

template<typename TestType>
static tainted<int, TestType> exampleCallback2( // NOLINT(google-runtime-int)
  RLBoxSandbox<TestType>& /* sandbox */,
  tainted<unsigned long, TestType> val1, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val2, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val3, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val4, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val5, // NOLINT(google-runtime-int)
  tainted<unsigned long, TestType> val6) // NOLINT(google-runtime-int)
{
  return ((val1.UNSAFE_Unverified() == 4)     // NOLINT
          && (val2.UNSAFE_Unverified() == 5)  // NOLINT
          && (val3.UNSAFE_Unverified() == 6)  // NOLINT
          && (val4.UNSAFE_Unverified() == 7)  // NOLINT
          && (val5.UNSAFE_Unverified() == 8)  // NOLINT
          && (val6.UNSAFE_Unverified() == 9)) // NOLINT
           ? 11                               // NOLINT
           : -1;                              // NOLINT
}

// NOLINTNEXTLINE
TEMPLATE_TEST_CASE("sandbox glue tests",
                   "[sandbox_glue_tests]",
                   rlbox::rlbox_noop_sandbox)
{
  rlbox::RLBoxSandbox<TestType> sandbox;
  sandbox.create_sandbox();

  const int upper_bound = 100;

  tainted<char*, TestType> sb_string =
    sandbox.template malloc_in_sandbox<char>(upper_bound);
  // strcpy is safe here
  // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
  std::strcpy(sb_string.UNSAFE_Unverified(), "Hello");

  SECTION("test simple function invocation") // NOLINT
  {
    const int val1 = 20;
    const int val2 = 22;
    tainted<int, TestType> a = val1;
    auto ret2 = sandbox_invoke(sandbox, simpleAddTest, a, val2);
    REQUIRE(ret2.UNSAFE_Unverified() == (val1 + val2));
  }

  SECTION("test 64 bit returns") // NOLINT
  {
    const uint32_t val1 = 20;
    const auto u32Max = std::numeric_limits<std::uint32_t>::max();
    auto ret2 = sandbox_invoke(sandbox, simpleU64AddTest, u32Max, val1);
    uint64_t result = static_cast<uint64_t>(u32Max) + val1;
    REQUIRE(ret2.UNSAFE_Unverified() == result);
  }

  SECTION("test verification function") // NOLINT
  {
    const int val1 = 2;
    const int val2 = 3;
    auto result1 = sandbox_invoke(sandbox, simpleAddTest, val1, val2)
                     .copy_and_verify(
                       [](int val) {
                         return val > 0 && val < upper_bound
                                  ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
                       },
                       -1);
    REQUIRE(result1 == val1 + val2);
  }

  SECTION("test pointer verification function") // NOLINT
  {
    const int val1 = 4;

    tainted<int*, TestType> pa = sandbox.template malloc_in_sandbox<int>();
    *pa = val1;

    auto result1 = sandbox_invoke(sandbox, echoPointer, pa)
                     .copy_and_verify(
                       [](const int* val) {
                         return *val > 0 && *val < upper_bound
                                  ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
                       },
                       -1);
    REQUIRE(result1 == val1);
  }

  SECTION("test callback 1 and re-entrancy") // NOLINT
  {
    const unsigned cb_val_param = 4;

    auto cb_callback_param =
      sandbox.register_callback(exampleCallback<rlbox::rlbox_noop_sandbox>);

    auto resultT = sandbox_invoke(
      sandbox, simpleCallbackTest, cb_val_param, sb_string, cb_callback_param);

    auto result = resultT.copy_and_verify(
      [](int val) {
        return val > 0 && val < upper_bound ? RLBox_Verify_Status::SAFE
                                            : RLBox_Verify_Status::UNSAFE;
      },
      -1);
    REQUIRE(result == 10);
  }

  SECTION("test callback 2") // NOLINT
  {
    auto cb_callback_param =
      sandbox.register_callback(exampleCallback2<rlbox::rlbox_noop_sandbox>);

    auto resultT =
      sandbox_invoke(sandbox, simpleCallbackTest2, 4, cb_callback_param);

    auto result = resultT.copy_and_verify(
      [](int /* val */) { return RLBox_Verify_Status::SAFE; }, -1);
    REQUIRE(result == 11);
  }

  SECTION("test callback to an internal function") // NOLINT
  {
    auto fnPtr = sandbox_function_address(sandbox, internalCallback);

    tainted<testStruct*, TestType> pFoo =
      sandbox.template malloc_in_sandbox<testStruct>();
    pFoo->fieldFnPtr = fnPtr;

    auto resultT = sandbox_invoke(
      sandbox, simpleCallbackTest, static_cast<unsigned>(4), sb_string, fnPtr);

    auto result = resultT.copy_and_verify(
      [](int val) {
        return val > 0 && val < upper_bound ? RLBox_Verify_Status::SAFE
                                            : RLBox_Verify_Status::UNSAFE;
      },
      -1);
    REQUIRE(result == 10);

    sandbox.free_in_sandbox(pFoo);
  }

  SECTION("test echo and pointer locations") // NOLINT
  {
    const char* str = "Hello";

    // str is allocated in our heap, not the sandbox's heap
    REQUIRE(sandbox.is_pointer_in_app_memory(str));

    tainted<char*, TestType> temp =
      sandbox.template malloc_in_sandbox<char>(std::strlen(str) + 1);
    char* str_in_sbx = temp.UNSAFE_Unverified();
    REQUIRE(sandbox.is_pointer_in_sandbox_memory(str_in_sbx));

    // strcpy is safe here
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
    std::strcpy(str_in_sbx, str);

    auto retStrRaw = sandbox_invoke(sandbox, simpleEchoTest, temp);
    // test some modifications of the string
    *retStrRaw = 'g';
    char* retStr = retStrRaw.copy_and_verify_string(
      [](char* val) {
        return strlen(val) < upper_bound ? RLBox_Verify_Status::SAFE
                                         : RLBox_Verify_Status::UNSAFE;
      },
      nullptr);

    REQUIRE(retStr != nullptr);
    REQUIRE(sandbox.is_pointer_in_app_memory(retStr));

    REQUIRE(strcmp(str, retStr) != 0);
    *retStrRaw = 'H';
    char* retStr2 = retStrRaw.copy_and_verify_string(
      [](char* val) {
        return strlen(val) < upper_bound ? RLBox_Verify_Status::SAFE
                                         : RLBox_Verify_Status::UNSAFE;
      },
      nullptr);
    REQUIRE(strcmp(str, retStr2) == 0);

    sandbox.free_in_sandbox(temp);
    delete[] retStr; // NOLINT
  }

  SECTION("test floating point") // NOLINT
  {
    const float fVal1 = 1.0;
    const float fVal2 = 2.0;
    const double dVal1 = 1.0;
    const double dVal2 = 2.0;
    const auto defaultVal = -1.0;

    auto resultF = sandbox_invoke(sandbox, simpleFloatAddTest, fVal1, fVal2)
                     .copy_and_verify(
                       [](float val) {
                         return val > 0 && val < upper_bound
                                  ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
                       },
                       defaultVal);
    REQUIRE(resultF == fVal1 + fVal2);

    auto resultD = sandbox_invoke(sandbox, simpleDoubleAddTest, dVal1, dVal2)
                     .copy_and_verify(
                       [](double val) {
                         return val > 0 && val < upper_bound
                                  ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
                       },
                       defaultVal);
    REQUIRE(resultD == dVal1 + dVal2);

    // test float to double conversions
    auto resultFD = sandbox_invoke(sandbox, simpleFloatAddTest, dVal1, dVal2)
                      .copy_and_verify(
                        [](double val) {
                          return val > 0 && val < upper_bound
                                   ? RLBox_Verify_Status::SAFE
                                   : RLBox_Verify_Status::UNSAFE;
                        },
                        defaultVal);
    REQUIRE(resultFD == dVal1 + dVal2);
  }

  SECTION("testPointerValAdd") // NOLINT
  {
    const double d1 = 1.0;
    const double d2 = 2.0;
    const auto defaultVal = -1.0;

    tainted<double*, TestType> p = sandbox.template malloc_in_sandbox<double>();
    *p = d1;

    auto resultD = sandbox_invoke(sandbox, simplePointerValAddTest, p, d2)
                     .copy_and_verify(
                       [](double val) {
                         return val > 0 && val < upper_bound
                                  ? RLBox_Verify_Status::SAFE
                                  : RLBox_Verify_Status::UNSAFE;
                       },
                       defaultVal);
    REQUIRE(resultD == d1 + d2);
  }

  // SECTION("testStructures" bool ignoreGlobalStringsInLib) // NOLINT
  // {
  //   auto resultT = sandbox_invoke(sandbox, simpleTestStructVal);
  //   auto result =
  //     resultT.copy_and_verify([](tainted<testStruct, TestType>& val) {
  //       testStruct ret{};
  //       ret.fieldLong =
  //         val.fieldLong.copy_and_verify([](unsigned long val) { return val;
  //         });
  //       ret.fieldString = ignoreGlobalStringsInLib
  //                           ? "Hello"
  //                           : val.fieldString.copy_and_verify_string(
  //                               sandbox,
  //                               [](const char* val) {
  //                                 return strlen(val) < upper_bound
  //                                          ? RLBox_Verify_Status::SAFE
  //                                          : RLBox_Verify_Status::UNSAFE;
  //                               },
  //                               nullptr);
  //       ret.fieldBool =
  //         val.fieldBool.copy_and_verify([](unsigned int val) { return val;
  //         });
  //       val.fieldFixedArr.copy_and_verify(ret.fieldFixedArr,
  //                                         sizeof(ret.fieldFixedArr),
  //                                         [](char* arr, size_t size) {
  //                                           UNUSED(arr);
  //                                           UNUSED(size);
  //                                           return RLBox_Verify_Status::SAFE;
  //                                         });
  //       return ret;
  //     });
  //   REQUIRE(result.fieldLong == 7 && strcmp(result.fieldString, "Hello") == 0
  //   &&
  //           result.fieldBool == 1 && strcmp(result.fieldFixedArr, "Bye") ==
  //           0);

  //   // writes should still go through
  //   resultT.fieldLong = 17;
  //   long val =
  //     resultT.fieldLong.copy_and_verify([](unsigned long val) { return val;
  //     });
  //   REQUIRE(val == 17);
  // }

  // SECTION("testStructurePointers"bool ignoreGlobalStringsInLib) // NOLINT
  // {
  //     auto resultT = sandbox_invoke(sandbox, simpleTestStructPtr);
  //     auto result = resultT
  //         .copy_and_verify([this,
  //         ignoreGlobalStringsInLib](tainted<testStruct, TestType>* val) {
  //             testStruct ret;
  //             ret.fieldLong = val->fieldLong.copy_and_verify([](unsigned long
  //             val) { return val; }); ret.fieldString =
  //             ignoreGlobalStringsInLib? "Hello" :
  //             val->fieldString.copy_and_verify_string(sandbox, [](const char*
  //             val) { return strlen(val) < upper_bound?
  //             RLBox_Verify_Status::SAFE : RLBox_Verify_Status::UNSAFE; },
  //             nullptr); ret.fieldBool =
  //             val->fieldBool.copy_and_verify([](unsigned int val) { return
  //             val;
  //             }); val->fieldFixedArr.copy_and_verify(ret.fieldFixedArr,
  //             sizeof(ret.fieldFixedArr), [](char* arr, size_t size){
  //             UNUSED(arr); UNUSED(size); return RLBox_Verify_Status::SAFE;
  //             }); return ret;
  //         });
  //     REQUIRE(result.fieldLong == 7 &&
  //         strcmp(result.fieldString, "Hello") == 0 &&
  //         result.fieldBool == 1 &&
  //         strcmp(result.fieldFixedArr, "Bye") == 0);

  //     //writes should still go through
  //     resultT->fieldLong = 17;
  //     long val2 = resultT->fieldLong.copy_and_verify([](unsigned long val) {
  //     return val; }); REQUIRE(val2 == 17);

  //     //test & and * operators
  //     unsigned long val3 = (*&resultT->fieldLong).copy_and_verify([](unsigned
  //     long val) { return val; }); REQUIRE(val3 == 17);
  // }

  // SECTION("test pointers in struct") // NOLINT
  // {
  //   auto initVal = sandbox.template malloc_in_sandbox<char>();
  //   auto resultT = sandbox_invoke(sandbox, initializePointerStruct, initVal);
  //   auto result =
  //     resultT.copy_and_verify([](tainted<pointersStruct, TestType>& val) {
  //       pointersStruct ret;
  //       ret.firstPointer = val.firstPointer.UNSAFE_Unverified();
  //       ret.pointerArray[0] = val.pointerArray[0].UNSAFE_Unverified();
  //       ret.pointerArray[1] = val.pointerArray[1].UNSAFE_Unverified();
  //       ret.pointerArray[2] = val.pointerArray[2].UNSAFE_Unverified();
  //       ret.pointerArray[3] = val.pointerArray[3].UNSAFE_Unverified();
  //       ret.lastPointer = val.lastPointer.UNSAFE_Unverified();
  //       return ret;
  //     });
  //   char* initValRaw = initVal.UNSAFE_Unverified();
  //   sandbox.free_in_sandbox(initVal);

  //   REQUIRE(result.firstPointer == initValRaw &&
  //           result.pointerArray[0] == (char*)(((uintptr_t)initValRaw) + 1) &&
  //           result.pointerArray[1] == (char*)(((uintptr_t)initValRaw) + 2) &&
  //           result.pointerArray[2] == (char*)(((uintptr_t)initValRaw) + 3) &&
  //           result.pointerArray[3] == (char*)(((uintptr_t)initValRaw) + 4) &&
  //           result.lastPointer == (char*)(((uintptr_t)initValRaw) + 5));
  // }

  SECTION("test 32-bit pointer edge cases") // NOLINT
  {
    auto initVal = sandbox.template malloc_in_sandbox<char>(upper_bound);
    auto initVal3 = initVal + 3; // NOLINT
    *initVal3 = 'v';

    auto resultT = sandbox_invoke(sandbox, initializePointerStructPtr, initVal);

    char* initValRaw = initVal.UNSAFE_Unverified();
    // check that reading a pointer in an array doesn't read neighboring
    // elements
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[0].UNSAFE_Unverified() == (initValRaw + 1));

    // check that a write doesn't overwrite neighboring elements
    resultT->pointerArray[0] = nullptr;
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[1].UNSAFE_Unverified() == (initValRaw + 2));

    // check that array reference decay followed by a read doesn't read
    // neighboring elements
    tainted<char**, TestType> elRef = &(resultT->pointerArray[2]);
    REQUIRE((**elRef).UNSAFE_Unverified() == 'v');

    // check that array reference decay followed by a write doesn't
    // overwrite neighboring elements
    *elRef = nullptr;
    // NOLINTNEXTLINE
    REQUIRE(resultT->pointerArray[3].UNSAFE_Unverified() == (initValRaw + 4));
  }

  sandbox.template free_in_sandbox(sb_string);

  sandbox.destroy_sandbox();
}