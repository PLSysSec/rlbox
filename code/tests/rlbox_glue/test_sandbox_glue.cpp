#include <cstdint>
#include <limits>
#include <memory>
namespace mpl_ {
struct na;
} // namespace mpl_

// IWYU pragma: begin_exports
#include "catch2/catch.hpp"
// IWYU pragma: end_exports

// NOLINTNEXTLINE
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
#define RLBOX_USE_EXCEPTIONS
#include "rlbox.hpp"
#include "rlbox_noop_sandbox.hpp"

#include "libtest.h"

using rlbox::tainted;

// NOLINTNEXTLINE
TEMPLATE_TEST_CASE("sandbox glue tests",
                   "[sandbox_glue_tests]",
                   rlbox::rlbox_noop_sandbox)
{
  rlbox::RLBoxSandbox<TestType> sandbox;
  sandbox.create_sandbox();

  const int int_val1 = 20;
  const int int_val2 = 22;

  SECTION("test simple function invocation") // NOLINT
  {
    tainted<int, TestType> a = int_val1;
    auto ret2 = sandbox_invoke(sandbox, simpleAddTest, a, int_val2);
    REQUIRE(ret2.UNSAFE_Unverified() == (int_val1 + int_val2));
  }

  SECTION("test 64 bit returns") // NOLINT
  {
    const auto u32Max = std::numeric_limits<std::uint32_t>::max();
    auto ret2 = sandbox_invoke(sandbox,
                               simpleU64AddTest,
                               u32Max,
                               int_val1);
    uint64_t result = static_cast<uint64_t>(u32Max) + int_val1;
    REQUIRE(ret2.UNSAFE_Unverified() == result);
  }

  // SECTION("testTwoVerificationFunctionFormats") // NOLINT
  // {
  //     auto result1 = sandbox_invoke(sandbox, simpleAddTest, 2, 3)
  //         .copyAndVerify([](int val){ return val > 0 && val < 10?
  //         RLBox_Verify_Status::SAFE : RLBox_Verify_Status::UNSAFE;}, -1);
  //     REQUIRE(result1 == 5);

  //     auto result2 = sandbox_invoke(sandbox, simpleAddTest, 2, 3)
  //         .copyAndVerify([](int val){ return val > 0 && val < 10? val :
  //         -1;});
  //     REQUIRE(result2 == 5);
  // }

  // SECTION("testPointerVerificationFunctionFormats") // NOLINT
  // {
  //     tainted<int*, TestType> pa = sandbox->template mallocInSandbox<int>();
  //     *pa = 4;

  //     auto result1 = sandbox_invoke(sandbox, echoPointer, pa)
  //         .copyAndVerify([](int* val){ return *val > 0 && *val < 10? *val :
  //         -1;});
  //     REQUIRE(result1 == 4);
  // }

  //     static int exampleCallback(RLBoxSandbox<TestType>* sandbox,
  //     tainted<unsigned, TestType> a, tainted<const char*, TestType> b,
  //     tainted<unsigned[1], TestType> c)
  // {
  //     auto aCopy = a.copyAndVerify([](unsigned val){ return val > 0 && val <
  //     100? val : -1; }); auto bCopy = b.copyAndVerifyString(sandbox, [](const
  //     char* val) { return strlen(val) < 100? RLBox_Verify_Status::SAFE :
  //     RLBox_Verify_Status::UNSAFE; }, nullptr); unsigned cCopy[1];
  //     c.copyAndVerify(cCopy, sizeof(cCopy), [](unsigned arr[1], size_t
  //     arrSize){
  //         UNUSED(arrSize);
  //         unsigned val = *arr;
  //         return val > 0 && val < 100? RLBox_Verify_Status::SAFE :
  //         RLBox_Verify_Status::UNSAFE;
  //     });
  //     if(cCopy[0] + 1 != aCopy)
  //     {
  //         printf("Unexpected callback value: %d, %d\n", cCopy[0] + 1, aCopy);
  //         exit(1);
  //     }
  //     auto ret = aCopy + strlen(bCopy);
  //     free((void*)bCopy);

  //     //test reentrancy
  //     tainted<int*, TestType> pFoo = sandbox->template
  //     mallocInSandbox<int>(); sandbox->freeInSandbox(pFoo); return ret;
  // }

  // static int exampleCallback2(RLBoxSandbox<TestType>* sandbox,
  //     tainted<unsigned long, TestType> val1,
  //     tainted<unsigned long, TestType> val2,
  //     tainted<unsigned long, TestType> val3,
  //     tainted<unsigned long, TestType> val4,
  //     tainted<unsigned long, TestType> val5,
  //     tainted<unsigned long, TestType> val6
  // )
  // {
  //     return (
  //         (val1.UNSAFE_Unverified() == 4) &&
  //         (val2.UNSAFE_Unverified() == 5) &&
  //         (val3.UNSAFE_Unverified() == 6) &&
  //         (val4.UNSAFE_Unverified() == 7) &&
  //         (val5.UNSAFE_Unverified() == 8) &&
  //         (val6.UNSAFE_Unverified() == 9)
  //     )? 11 : -1;
  // }

  // SECTION("testCallback") // NOLINT
  // {
  //     {
  //         auto resultT = sandbox_invoke(sandbox, simpleCallbackTest,
  //         (unsigned) 4, sandbox->stackarr("Hello"), registeredCallback);

  //         auto result = resultT
  //             .copyAndVerify([](int val){ return val > 0 && val < 100? val :
  //             -1; });
  //         REQUIRE(result == 10);
  //     }

  //     {
  //         auto resultT = sandbox_invoke(sandbox, simpleCallbackTest2, 4,
  //         registeredCallback2);

  //         auto result = resultT
  //             .copyAndVerify([](int val){ return val; });
  //         REQUIRE(result == 11);
  //     }
  // }

  // SECTION("testInternalCallback") // NOLINT
  // {
  //     auto fnPtr = sandbox_function(sandbox, internalCallback);

  //     tainted<testStruct*, TestType> pFoo = sandbox->template
  //     mallocInSandbox<testStruct>(); pFoo->fieldFnPtr = fnPtr;

  //     auto resultT = sandbox_invoke(sandbox, simpleCallbackTest, (unsigned)
  //     4, sandbox->stackarr("Hello"), fnPtr); auto result = resultT
  //         .copyAndVerify([](int val){ return val > 0 && val < 100? val : -1;
  //         });
  //     REQUIRE(result == 10);
  // }

  // SECTION("testEchoAndPointerLocations") // NOLINT
  // {
  //     const char* str = "Hello";

  //     //str is allocated in our heap, not the sandbox's heap
  //     REQUIRE(sandbox->isPointerInAppMemoryOrNull(str));

  //     tainted<char*, TestType> temp = sandbox->template
  //     mallocInSandbox<char>(strlen(str) + 1); char* strInSandbox =
  //     temp.UNSAFE_Unverified();
  //     REQUIRE(sandbox->isPointerInSandboxMemoryOrNull(strInSandbox));

  //     strcpy(strInSandbox, str);

  //     auto retStrRaw = sandbox_invoke(sandbox, simpleEchoTest, temp);
  //     *retStrRaw = 'g';
  //     *retStrRaw = 'H';
  //     char* retStr = retStrRaw.copyAndVerifyString(sandbox, [](char* val) {
  //     return strlen(val) < 100? RLBox_Verify_Status::SAFE :
  //     RLBox_Verify_Status::UNSAFE; }, nullptr);

  //     REQUIRE(retStr != nullptr &&
  //     sandbox->isPointerInAppMemoryOrNull(retStr));

  //     auto isStringSame = strcmp(str, retStr) == 0;
  //     REQUIRE(isStringSame);

  //     sandbox->freeInSandbox(temp);
  //     free(retStr);
  // }

  // SECTION("testFloatingPoint") // NOLINT
  // {
  //     auto resultF = sandbox_invoke(sandbox, simpleFloatAddTest, 1.0f, 2.0f)
  //         .copyAndVerify([](float val){ return val > 0 && val < 100? val :
  //         -1.0; });
  //     REQUIRE(resultF == 3.0);

  //     auto resultD = sandbox_invoke(sandbox, simpleDoubleAddTest, 1.0, 2.0)
  //         .copyAndVerify([](double val){ return val > 0 && val < 100? val :
  //         -1.0; });
  //     REQUIRE(resultD == 3.0);

  //     //test float to double conversions

  //     auto resultFD = sandbox_invoke(sandbox, simpleFloatAddTest, 1.0, 2.0)
  //         .copyAndVerify([](double val){ return val > 0 && val < 100? val :
  //         -1.0; });
  //     REQUIRE(resultFD == 3.0);
  // }

  // SECTION("testPointerValAdd") // NOLINT
  // {
  //     tainted<double*, TestType> pd = sandbox->template
  //     mallocInSandbox<double>(); *pd = 1.0;

  //     double d = 2.0;

  //     auto resultD = sandbox_invoke(sandbox, simplePointerValAddTest, pd, d)
  //         .copyAndVerify([](double val){ return val > 0 && val < 100? val :
  //         -1.0; });
  //     REQUIRE(resultD == 3.0);
  // }

  // SECTION("testStructures"bool ignoreGlobalStringsInLib) // NOLINT
  // {
  //     auto resultT = sandbox_invoke(sandbox, simpleTestStructVal);
  //     auto result = resultT
  //         .copyAndVerify([this, ignoreGlobalStringsInLib](tainted<testStruct,
  //         TestType>& val){
  //             testStruct ret;
  //             ret.fieldLong = val.fieldLong.copyAndVerify([](unsigned long
  //             val) { return val; }); ret.fieldString =
  //             ignoreGlobalStringsInLib? "Hello" :
  //             val.fieldString.copyAndVerifyString(sandbox, [](const char*
  //             val) { return strlen(val) < 100? RLBox_Verify_Status::SAFE :
  //             RLBox_Verify_Status::UNSAFE; }, nullptr); ret.fieldBool =
  //             val.fieldBool.copyAndVerify([](unsigned int val) { return val;
  //             }); val.fieldFixedArr.copyAndVerify(ret.fieldFixedArr,
  //             sizeof(ret.fieldFixedArr), [](char* arr, size_t size){
  //             UNUSED(arr); UNUSED(size); return RLBox_Verify_Status::SAFE;
  //             }); return ret;
  //         });
  //     REQUIRE(result.fieldLong == 7 &&
  //         strcmp(result.fieldString, "Hello") == 0 &&
  //         result.fieldBool == 1 &&
  //         strcmp(result.fieldFixedArr, "Bye") == 0);

  //     //writes should still go through
  //     resultT.fieldLong = 17;
  //     long val = resultT.fieldLong.copyAndVerify([](unsigned long val) {
  //     return val; }); REQUIRE(val == 17);
  // }

  // SECTION("testStructurePointers"bool ignoreGlobalStringsInLib) // NOLINT
  // {
  //     auto resultT = sandbox_invoke(sandbox, simpleTestStructPtr);
  //     auto result = resultT
  //         .copyAndVerify([this, ignoreGlobalStringsInLib](tainted<testStruct,
  //         TestType>* val) {
  //             testStruct ret;
  //             ret.fieldLong = val->fieldLong.copyAndVerify([](unsigned long
  //             val) { return val; }); ret.fieldString =
  //             ignoreGlobalStringsInLib? "Hello" :
  //             val->fieldString.copyAndVerifyString(sandbox, [](const char*
  //             val) { return strlen(val) < 100? RLBox_Verify_Status::SAFE :
  //             RLBox_Verify_Status::UNSAFE; }, nullptr); ret.fieldBool =
  //             val->fieldBool.copyAndVerify([](unsigned int val) { return val;
  //             }); val->fieldFixedArr.copyAndVerify(ret.fieldFixedArr,
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
  //     long val2 = resultT->fieldLong.copyAndVerify([](unsigned long val) {
  //     return val; }); REQUIRE(val2 == 17);

  //     //test & and * operators
  //     unsigned long val3 = (*&resultT->fieldLong).copyAndVerify([](unsigned
  //     long val) { return val; }); REQUIRE(val3 == 17);
  // }

  // SECTION("testPointersInStruct") // NOLINT
  // {
  //     auto initVal = sandbox->template mallocInSandbox<char>();
  //     auto resultT = sandbox_invoke(sandbox, initializePointerStruct,
  //     initVal); auto result = resultT
  //         .copyAndVerify([](tainted<pointersStruct, TestType>& val){
  //             pointersStruct ret;
  //             ret.firstPointer = val.firstPointer.UNSAFE_Unverified();
  //             ret.pointerArray[0] = val.pointerArray[0].UNSAFE_Unverified();
  //             ret.pointerArray[1] = val.pointerArray[1].UNSAFE_Unverified();
  //             ret.pointerArray[2] = val.pointerArray[2].UNSAFE_Unverified();
  //             ret.pointerArray[3] = val.pointerArray[3].UNSAFE_Unverified();
  //             ret.lastPointer = val.lastPointer.UNSAFE_Unverified();
  //             return ret;
  //         });
  //     char* initValRaw = initVal.UNSAFE_Unverified();
  //     sandbox->freeInSandbox(initVal);

  //     REQUIRE(
  //         result.firstPointer == initValRaw &&
  //         result.pointerArray[0] == (char*) (((uintptr_t) initValRaw) + 1) &&
  //         result.pointerArray[1] == (char*) (((uintptr_t) initValRaw) + 2) &&
  //         result.pointerArray[2] == (char*) (((uintptr_t) initValRaw) + 3) &&
  //         result.pointerArray[3] == (char*) (((uintptr_t) initValRaw) + 4) &&
  //         result.lastPointer ==     (char*) (((uintptr_t) initValRaw) + 5)
  //     );
  // }

  // SECTION("test32BitPointerEdgeCases") // NOLINT
  // {
  //     auto initVal = sandbox->template mallocInSandbox<char>(8);
  //     *(initVal.getPointerIncrement(sandbox, 3)) = 'v';
  //     char* initValRaw = initVal.UNSAFE_Unverified();

  //     auto resultT = sandbox_invoke(sandbox, initializePointerStructPtr,
  //     initVal);

  //     //check that reading a pointer in an array doesn't read neighboring
  //     elements REQUIRE(
  //         resultT->pointerArray[0].UNSAFE_Unverified() == (char*)
  //         (((uintptr_t) initValRaw) + 1)
  //     );

  //     //check that a write doesn't overwrite neighboring elements
  //     resultT->pointerArray[0] = nullptr;
  //     REQUIRE(
  //         resultT->pointerArray[1].UNSAFE_Unverified() == (char*)
  //         (((uintptr_t) initValRaw) + 2)
  //     );

  //     //check that array reference decay followed by a read doesn't read
  //     neighboring elements tainted<char**, TestType> elRef =
  //     &(resultT->pointerArray[2]); REQUIRE((**elRef).UNSAFE_Unverified() ==
  //     'v');

  //     //check that array reference decay followed by a write doesn't
  //     overwrite neighboring elements *elRef = nullptr; REQUIRE(
  //         resultT->pointerArray[3].UNSAFE_Unverified() == (char*)
  //         (((uintptr_t) initValRaw) + 4)
  //     );
  // }

  sandbox.destroy_sandbox();
}