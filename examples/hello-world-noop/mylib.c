#include "mylib.h"

#include <stdio.h>

void hello()
{
  printf("Hello world from mylib\n");
}

void echo(const char* str)
{
  printf("> mylib: %s\n", str);
}

unsigned add(unsigned a, unsigned b)
{
  return a + b;
}

void call_cb(void (*cb) (const char* str))
{
  cb("hi again!");
}
