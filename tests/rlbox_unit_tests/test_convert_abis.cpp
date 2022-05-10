/**
 * @file test_convert_abis.cpp
 * @brief Check that ABI conversion works as expected.
 * @details This file tests the following things:
 * 1. The `copy_cvref_t` trait which is used by `convert_base_types_t`
 * 2. The `convert_base_types_t` used to convert non pointer types
 * 3. The `convert_base_types_t` used to convert pointer types when:
 *      - The pointer types are converted to `void*`
 *      - The pointer types are converted to `int`
 *      - The pointer types are converted to `struct`
 */

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_type_traits.hpp"
#include "test_include.hpp"

///////////////////////////////////////////////////////////////////////////
// Part 1. The `copy_cvref_t` trait which is used by `convert_base_types_t`
///////////////////////////////////////////////////////////////////////////

TEST_CASE("Test cvref preservation", "[abi conversion]")
{
  REQUIRE(std::is_same_v<int, detail::copy_cvref_t<char, int>>);
  REQUIRE(std::is_same_v<const int, detail::copy_cvref_t<const char, int>>);
  REQUIRE(
    std::is_same_v<volatile int, detail::copy_cvref_t<volatile char, int>>);
  REQUIRE(std::is_same_v<int&, detail::copy_cvref_t<char&, int>>);
  REQUIRE(std::is_same_v<int&&, detail::copy_cvref_t<char&&, int>>);
  REQUIRE(std::is_same_v<const volatile int&,
                         detail::copy_cvref_t<const volatile char&, int>>);
}

///////////////////////////////////////////////////////////////////////////
// Part 2. The `convert_base_types_t` used to convert non pointer types
///////////////////////////////////////////////////////////////////////////

using test_wchar = uint64_t;
using test_short = int;
using test_int = long;
using test_long = long long;
using test_longlong = short;
using test_size_t = size_t;
using test_pointer = void*;

template<typename T, typename TPtr>
using test_convertor_except_ptr = detail::convert_base_types_t<T,
                                                               test_wchar,
                                                               test_short,
                                                               test_int,
                                                               test_long,
                                                               test_longlong,
                                                               test_size_t,
                                                               TPtr>;

template<typename T>
using test_convertor = test_convertor_except_ptr<T, test_pointer>;

// First test with standard primitive and array C++ types

TEST_CASE("Test ABI conversion of primitive unchanged types",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<void, test_convertor<void>>);
  REQUIRE(std::is_same_v<bool, test_convertor<bool>>);
  REQUIRE(std::is_same_v<char, test_convertor<char>>);
  REQUIRE(std::is_same_v<signed char, test_convertor<signed char>>);
  REQUIRE(std::is_same_v<unsigned char, test_convertor<unsigned char>>);
#if __cplusplus >= 202002L
  REQUIRE(std::is_same_v<char8_t, test_convertor<char8_t>>);
#endif
  REQUIRE(std::is_same_v<char16_t, test_convertor<char16_t>>);
  REQUIRE(std::is_same_v<char32_t, test_convertor<char32_t>>);
  REQUIRE(std::is_same_v<char32_t, test_convertor<char32_t>>);
  REQUIRE(std::is_same_v<float, test_convertor<float>>);
  REQUIRE(std::is_same_v<double, test_convertor<double>>);
  REQUIRE(std::is_same_v<long double, test_convertor<long double>>);
}

TEST_CASE("Test ABI conversion of primitive changed nosign types",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<test_wchar, test_convertor<wchar_t>>);
}

TEST_CASE("Test ABI conversion of primitive changed signed types",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<test_short, test_convertor<short>>);
  REQUIRE(std::is_same_v<test_int, test_convertor<int>>);
  REQUIRE(std::is_same_v<test_long, test_convertor<long>>);
  REQUIRE(std::is_same_v<test_longlong, test_convertor<long long>>);
}

TEST_CASE("Test ABI conversion of primitive changed unsigned types",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_short>,
                         test_convertor<unsigned short>>);
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_int>,
                         test_convertor<unsigned int>>);
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_long>,
                         test_convertor<unsigned long>>);
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_longlong>,
                         test_convertor<unsigned long long>>);
}

TEST_CASE("Test ABI conversion of array types", "[abi conversion]")
{
  REQUIRE(std::is_same_v<test_short[3], test_convertor<short[3]>>);
}

// Next with qualifiers for each of the cases above

TEST_CASE("Test ABI conversion with const qualifier", "[abi conversion]")
{
  REQUIRE(std::is_same_v<const char, test_convertor<const char>>);
  REQUIRE(std::is_same_v<const test_wchar, test_convertor<const wchar_t>>);
  REQUIRE(std::is_same_v<const test_short, test_convertor<const short>>);
  REQUIRE(std::is_same_v<const std::make_unsigned_t<test_short>,
                         test_convertor<const unsigned short>>);
  REQUIRE(std::is_same_v<const test_short[3], test_convertor<const short[3]>>);
}

TEST_CASE("Test ABI conversion of with volatile qualifier", "[abi conversion]")
{
  REQUIRE(std::is_same_v<volatile char, test_convertor<volatile char>>);
  REQUIRE(
    std::is_same_v<volatile test_wchar, test_convertor<volatile wchar_t>>);
  REQUIRE(std::is_same_v<volatile test_short, test_convertor<volatile short>>);
  REQUIRE(std::is_same_v<volatile std::make_unsigned_t<test_short>,
                         test_convertor<volatile unsigned short>>);
  REQUIRE(
    std::is_same_v<volatile test_short[3], test_convertor<volatile short[3]>>);
}

TEST_CASE("Test ABI conversion of with lreference qualifier",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<char&, test_convertor<char&>>);
  REQUIRE(std::is_same_v<test_wchar&, test_convertor<wchar_t&>>);
  REQUIRE(std::is_same_v<test_short&, test_convertor<short&>>);
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_short>&,
                         test_convertor<unsigned short&>>);
}

TEST_CASE("Test ABI conversion of with rreference qualifier",
          "[abi conversion]")
{
  REQUIRE(std::is_same_v<char&&, test_convertor<char&&>>);
  REQUIRE(std::is_same_v<test_wchar&&, test_convertor<wchar_t&&>>);
  REQUIRE(std::is_same_v<test_short&&, test_convertor<short&&>>);
  REQUIRE(std::is_same_v<std::make_unsigned_t<test_short>&&,
                         test_convertor<unsigned short&&>>);
}

TEST_CASE("Test ABI conversion with multiple qualifiers", "[abi conversion]")
{
  REQUIRE(
    std::is_same_v<const volatile char&, test_convertor<const volatile char&>>);
  REQUIRE(std::is_same_v<const volatile test_wchar&,
                         test_convertor<const volatile wchar_t&>>);
  REQUIRE(std::is_same_v<const volatile test_short&&,
                         test_convertor<const volatile short&&>>);
  REQUIRE(std::is_same_v<const volatile std::make_unsigned_t<test_short>&&,
                         test_convertor<const volatile unsigned short&&>>);
  REQUIRE(std::is_same_v<const volatile test_short[3],
                         test_convertor<const volatile short[3]>>);
}

///////////////////////////////////////////////////////////////////////////
// Part 3. The `convert_base_types_t` used to convert pointer types when:
//          - The pointer types are converted to `void*`
///////////////////////////////////////////////////////////////////////////

TEST_CASE("Test ABI conversion of pointers if destination ABI uses void*",
          "[abi conversion]")
{
  static_assert(std::is_same_v<test_pointer, void*>);

  // Test ABI conversion of changed nosign types
  REQUIRE(std::is_same_v<void*, test_convertor<void*>>);

  // Test ABI conversion of pointer types
  REQUIRE(std::is_same_v<void*, test_convertor<short*>>);

  // Test ABI conversion of const types
  REQUIRE(std::is_same_v<const void*, test_convertor<const short*>>);
  REQUIRE(std::is_same_v<void* const, test_convertor<short* const>>);
  REQUIRE(
    std::is_same_v<const void* const, test_convertor<const short* const>>);

  // Test ABI conversion of volatile types
  REQUIRE(std::is_same_v<volatile void*, test_convertor<volatile short*>>);
  REQUIRE(std::is_same_v<void* volatile, test_convertor<short* volatile>>);
  REQUIRE(std::is_same_v<volatile void* volatile,
                         test_convertor<volatile short* volatile>>);

  // Test ABI conversion of lreference types
  // Arrays of references not supported in C++
  // Pointer to a reference not supported in C++
  REQUIRE(std::is_same_v<void*&, test_convertor<short*&>>);

  // Test ABI conversion of rreference types
  // Arrays of references not supported in C++
  // Pointer to a reference not supported in C++
  REQUIRE(std::is_same_v<void*&&, test_convertor<short*&&>>);

  // Test ABI conversion of multiple qualifiers
  REQUIRE(std::is_same_v<const volatile void* const&,
                         test_convertor<const volatile short* const&>>);
}

///////////////////////////////////////////////////////////////////////////
// Part 3. The `convert_base_types_t` used to convert pointer types when:
//          - The pointer types are converted to `int`
//          - The pointer types are converted to `struct`
///////////////////////////////////////////////////////////////////////////

template<typename TPtr, template<typename TType> class TConv>
static void test_ptr_convert_helper()
{
  static_assert(!std::is_pointer_v<TPtr>);

  // Test ABI conversion of changed nosign types
  REQUIRE(std::is_same_v<TPtr, TConv<void*>>);

  // Test ABI conversion of pointer types
  REQUIRE(std::is_same_v<TPtr, TConv<short*>>);

  // Test ABI conversion of const types
  REQUIRE(std::is_same_v<TPtr, TConv<const short*>>);
  REQUIRE(std::is_same_v<const TPtr, TConv<short* const>>);
  REQUIRE(std::is_same_v<const TPtr, TConv<const short* const>>);

  // Test ABI conversion of volatile types
  REQUIRE(std::is_same_v<TPtr, TConv<volatile short*>>);
  REQUIRE(std::is_same_v<volatile TPtr, TConv<short* volatile>>);
  REQUIRE(std::is_same_v<volatile TPtr, TConv<volatile short* volatile>>);

  // Test ABI conversion of lreference types
  // Arrays of references not supported in C++
  // Pointer to a reference not supported in C++
  REQUIRE(std::is_same_v<TPtr&, TConv<short*&>>);

  // Test ABI conversion of rreference types
  // Arrays of references not supported in C++
  // Pointer to a reference not supported in C++
  REQUIRE(std::is_same_v<TPtr&&, TConv<short*&&>>);

  // Test ABI conversion of multiple qualifiers
  REQUIRE(std::is_same_v<const TPtr&, TConv<const volatile short* const&>>);
}

using test_pointer_int = int;

template<typename T>
using test_convertor_int_ptr = test_convertor_except_ptr<T, test_pointer_int>;

using test_pointer_struct = struct
{
  int a;
};

template<typename T>
using test_convertor_struct_ptr =
  test_convertor_except_ptr<T, test_pointer_struct>;

TEST_CASE(
  "Test ABI conversion of pointers if destination ABI uses a int/struct",
  "[abi conversion]")
{
  test_ptr_convert_helper<test_pointer_int, test_convertor_int_ptr>();
  test_ptr_convert_helper<test_pointer_struct, test_convertor_struct_ptr>();
}
