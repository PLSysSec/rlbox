#include "libtest.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

unsigned long simpleAddNoPrintTest(unsigned long a, unsigned long b)
{
  return a + b;
}

unsigned long simpleCallbackLoop(unsigned long a,
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

float callbackTypeFloatTest(float val, CallbackTypeFloat callback)
{
  return callback(val);
}

double callbackTypeDoubleTest(double val, CallbackTypeDouble callback)
{
  return callback(val);
}

long long int callbackTypeLongLongTest(long long int val,
                                       CallbackTypeLongLong callback)
{
  return callback(val);
}

double simpleDivideTest(double a, double b)
{
  return a / b;
}

int simpleAddTest(int a, int b)
{
  printf("simpleAddTest\n");
  fflush(stdout);
  return a + b;
}

size_t simpleStrLenTest(const char* str)
{
  printf("simpleStrLenTest\n");
  fflush(stdout);
  return strlen(str);
}

int simpleCallbackTest(unsigned a, const char* b, CallbackType callback)
{
  int ret;

  printf("simpleCallbackTest\n");
  fflush(stdout);

  ret = callback(a + 1, b, &a);
  return ret;
}

int simpleWriteToFileTest(FILE* file, const char* str)
{
  printf("simpleWriteToFileTest\n");
  fflush(stdout);
  return fputs(str, file);
}

char* simpleEchoTest(char* str)
{
  printf("simpleEchoTest\n");
  fflush(stdout);
  return str;
}

float simpleFloatAddTest(const float a, const float b)
{
  printf("simpleFloatAddTest\n");
  return a + b;
}

double simpleDoubleAddTest(const double a, const double b)
{
  printf("simpleDoubleAddTest\n");
  return a + b;
}

unsigned long simpleLongAddTest(unsigned long a, unsigned long b)
{
  printf("simpleLongAddTest\n");
  fflush(stdout);
  return a + b;
}

struct testStruct simpleTestStructVal()
{
  struct testStruct ret;
  ret.fieldLong = 7;
  ret.fieldString = "Hello";
  ret.fieldBool = 1;
  strcpy(ret.fieldFixedArr, "Bye");
  return ret;
}

struct testStruct* simpleTestStructPtr()
{
  struct testStruct* ret =
    (struct testStruct*)malloc(sizeof(struct testStruct));
  ret->fieldLong = 7;
  ret->fieldString = "Hello";
  ret->fieldBool = 1;
  strcpy(ret->fieldFixedArr, "Bye");
  return ret;
}

struct testStruct simpleTestStructValBadPtr()
{
  struct testStruct ret = simpleTestStructVal();
  // explicitly mess up the top bits of the pointer. The sandbox checks outside
  // the sandbox should catch this
  if (sizeof(void*) == 4) {
    ret.fieldString =
      (char*)((((uintptr_t)ret.fieldString) & 0x3FFFFFFF) | 0xC0000000);
  } else {
    ret.fieldString =
      (char*)((((uintptr_t)ret.fieldString) & 0xFFFFFFFF) | 0x1234567800000000);
  }
  return ret;
}

struct testStruct* simpleTestStructPtrBadPtr()
{
  struct testStruct* ret = simpleTestStructPtr();
  // explicitly mess up the top bits of the pointer. The sandbox checks outside
  // the sandbox should catch this
  if (sizeof(void*) == 4) {
    ret->fieldString =
      (char*)((((uintptr_t)ret->fieldString) & 0x3FFFFFFF) | 0xC0000000);
  } else {
    ret->fieldString = (char*)((((uintptr_t)ret->fieldString) & 0xFFFFFFFF) |
                               0x1234567800000000);
  }
  return ret;
}

long simpleTestStructParam(struct testStruct param)
{
  return (long) param.fieldLong + (long) (param.fieldString? strlen(param.fieldString) : 0);
}

int* echoPointer(int* pointer)
{
  return pointer;
}

double simplePointerValAddTest(double* ptr, double val)
{
  printf("simplePointerValAddTest\n");
  return val + *ptr;
}

struct pointersStruct initializePointerStruct(char* initVal)
{
  struct pointersStruct ret;
  ret.firstPointer = initVal;
  ret.pointerArray[0] = (char*)(((uintptr_t)initVal) + 1);
  ret.pointerArray[1] = (char*)(((uintptr_t)initVal) + 2);
  ret.pointerArray[2] = (char*)(((uintptr_t)initVal) + 3);
  ret.pointerArray[3] = (char*)(((uintptr_t)initVal) + 4);
  ret.lastPointer = (char*)(((uintptr_t)initVal) + 5);
  return ret;
}

struct pointersStruct* initializePointerStructPtr(char* initVal)
{
  struct pointersStruct* ret =
    (struct pointersStruct*)malloc(sizeof(struct pointersStruct));
  ret->firstPointer = initVal;
  ret->pointerArray[0] = (char*)(((uintptr_t)initVal) + 1);
  ret->pointerArray[1] = (char*)(((uintptr_t)initVal) + 2);
  ret->pointerArray[2] = (char*)(((uintptr_t)initVal) + 3);
  ret->pointerArray[3] = (char*)(((uintptr_t)initVal) + 4);
  ret->lastPointer = (char*)(((uintptr_t)initVal) + 5);
  return ret;
}

int internalCallback(unsigned a, const char* b, unsigned c[1])
{
  (void)c;
  return (int) a + (int) strlen(b);
}

void simplePointerWrite(int* ptr, int val)
{
  *ptr = val;
}

int simpleCallbackTest2(unsigned long startVal, CallbackType2 cb)
{
  return cb(startVal,
            startVal + 1,
            startVal + 2,
            startVal + 3,
            startVal + 4,
            startVal + 5);
}

unsigned long stackParametersTest(unsigned long a1,
                                  unsigned long a2,
                                  unsigned long a3,
                                  unsigned long a4,
                                  unsigned long a5,
                                  unsigned long a6,
                                  unsigned long a7,
                                  unsigned long a8,
                                  unsigned long a9)
{
  return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
}

int isNonNullChar(unsigned char p)
{
  if (p) {
    return 1;
  } else {
    return 0;
  }
}
