#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__) || defined(__GNUG__)
// Can't turn off the variadic macro warning emitted from -pedantic
#  pragma GCC system_header
#elif defined(_MSC_VER)
// Doesn't seem to emit the warning
#else
// Don't know the compiler... just let it go through
#endif

// clang-format off
#define sandbox_fields_reflection_libtest_class_testStruct(f, g, ...)          \
  f(unsigned long, fieldLong, FIELD_NORMAL, ##__VA_ARGS__) g()                 \
  f(const char*, fieldString, FIELD_NORMAL, ##__VA_ARGS__) g()                 \
  f(unsigned int, fieldBool, FIELD_NORMAL, ##__VA_ARGS__) g()                  \
  f(char[8], fieldFixedArr, FIELD_NORMAL, ##__VA_ARGS__) g()                   \
  f(int (*)(unsigned, const char*, unsigned[1]),                               \
    fieldFnPtr, FIELD_NORMAL, ##__VA_ARGS__) g()                               \
  f(struct unknownClass*, fieldUnknownPtr, FIELD_NORMAL, ##__VA_ARGS__) g()    \
  f(void*, voidPtr, FIELD_NORMAL, ##__VA_ARGS__) g()                           \
  f(int (*[4])(unsigned, const char*, unsigned[1]),                            \
    fnArray, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_libtest_class_frozenStruct(f, g, ...)        \
  f(int, normalField, FIELD_NORMAL, ##__VA_ARGS__) g()                         \
  f(int, fieldForFreeze, FIELD_FREEZABLE, ##__VA_ARGS__) g()                   \
  f(struct frozenStructTest*, next, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_libtest_class_pointersStruct(f, g, ...)      \
  f(char*, firstPointer, FIELD_NORMAL, ##__VA_ARGS__) g()                      \
  f(char * [4], pointerArray, FIELD_NORMAL, ##__VA_ARGS__) g()                 \
  f(char*, lastPointer, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_libtest_allClasses(f, ...)                   \
  f(testStruct, libtest, ##__VA_ARGS__)                                        \
  f(frozenStruct, libtest, ##__VA_ARGS__)                                      \
  f(pointersStruct, libtest, ##__VA_ARGS__)

// clang-format on

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#elif defined(_MSC_VER)
#else
#endif
