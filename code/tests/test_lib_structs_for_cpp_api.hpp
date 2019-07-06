#define sandbox_fields_reflection_testlib_class_testStruct(f, g, ...)          \
  f(unsigned long, fieldLong, FIELD_NORMAL, ##__VA_ARGS__) g()                 \
    f(const char*, fieldString, FIELD_NORMAL, ##__VA_ARGS__) g() f(            \
      unsigned int, fieldBool, FIELD_NORMAL, ##__VA_ARGS__) g()                \
      f(char[8], fieldFixedArr, FIELD_NORMAL, ##__VA_ARGS__) g() f(            \
        int (*)(unsigned, const char*, unsigned[1]),                           \
        fieldFnPtr,                                                            \
        FIELD_NORMAL,                                                          \
        ##__VA_ARGS__) g()                                                     \
        f(struct unknownClass*, fieldUnknownPtr, FIELD_NORMAL, ##__VA_ARGS__)  \
          g() f(void*, voidPtr, FIELD_NORMAL, ##__VA_ARGS__) g()               \
            f(int (*[8])(unsigned, const char*, unsigned[1]),                  \
              fnArray,                                                         \
              FIELD_NORMAL,                                                    \
              ##__VA_ARGS__) g()

#define sandbox_fields_reflection_testlib_allClasses(f, ...)                   \
  f(testStruct, testlib, ##__VA_ARGS__)
