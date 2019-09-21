#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
  typedef int (*CallbackType)(unsigned, const char*, unsigned[1]);
  typedef int (*CallbackType2)(unsigned long,
                               unsigned long,
                               unsigned long,
                               unsigned long,
                               unsigned long,
                               unsigned long);
  typedef unsigned long (*CallbackType3)(unsigned long, unsigned long);

  struct testStruct
  {
    unsigned long fieldLong;
    const char* fieldString;
    unsigned int fieldBool;
    char fieldFixedArr[8];
    int (*fieldFnPtr)(unsigned, const char*, unsigned[1]);
    struct unknownClass* fieldUnknownPtr;
    void* voidPtr;
    CallbackType fnArray[4];
  };

  struct frozenStruct
  {
    int normalField;
    int fieldForFreeze;
    struct frozenStructTest* next;
  };

  struct pointersStruct
  {
    char* firstPointer;
    char* pointerArray[4];
    char* lastPointer;
  };

  unsigned long simpleAddNoPrintTest(unsigned long a, unsigned long b);
  unsigned long simpleCallbackLoop(unsigned long a,
                                   unsigned long b,
                                   unsigned long iterations,
                                   CallbackType3 callback);
  double simpleDivideTest(double a, double b);
  int simpleAddTest(int a, int b);
  size_t simpleStrLenTest(const char* str);
  int simpleCallbackTest(unsigned a, const char* b, CallbackType callback);
  int simpleWriteToFileTest(FILE* file, const char* str);
  char* simpleEchoTest(char* str);
  float simpleFloatAddTest(const float a, const float b);
  double simpleDoubleAddTest(const double a, const double b);
  unsigned long simpleLongAddTest(unsigned long a, unsigned long b);
  struct testStruct simpleTestStructVal();
  struct testStruct* simpleTestStructPtr();
  struct testStruct simpleTestStructValBadPtr();
  struct testStruct* simpleTestStructPtrBadPtr();
  int* echoPointer(int* pointer);
  double simplePointerValAddTest(double* ptr, double val);
  struct pointersStruct initializePointerStruct(char* initVal);
  struct pointersStruct* initializePointerStructPtr(char* initVal);
  int internalCallback(unsigned, const char*, unsigned[1]);
  void simplePointerWrite(int* ptr, int val);
  int simpleCallbackTest2(unsigned long startVal, CallbackType2 cb);
#ifdef __cplusplus
}
#endif
