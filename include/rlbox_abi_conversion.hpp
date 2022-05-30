/**
 * @file rlbox_abi_conversion.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief The file provides utilities to convert between different ABIs
 * @details In particular we can convert data structures, between 2 ABIs the
 * source ABI to a target ABI. Each ABI can specify custom sizes of short,
 * int, long, and pointer sizes. Thus the type `int` may have different sizes in
 * the source and target ABI. This file provides a trait allows conversions
 * of these types from the source to the target ABI.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include "rlbox_stdint_types.hpp"
#include "rlbox_type_traits.hpp"

/**
 * @brief abi_template_decls is the list of types that can be converted by this
 * ABI convertor
 */
#define abi_template_decls                                            \
  typename TWCharType, typename TShortType, typename TIntType,        \
      typename TLongType, typename TLongLongType, typename TSizeType, \
      typename TPointerType

/**
 * @brief abi_template_names is the list of typenames in abi_template_decls
 */
#define abi_template_names                                               \
  TWCharType, TShortType, TIntType, TLongType, TLongLongType, TSizeType, \
      TPointerType

namespace rlbox::detail {

namespace convert_base_types_detail {

/**
 * @brief This trait is the internal helper to implement convert_base_types_t
 *
 * @tparam T is the type to be converted
 * @tparam abi_template_decls is the target ABI
 * @tparam TEnable is the dummy template parameter that allows us to specialize
 * the implementation for different types
 */
template <typename T, abi_template_decls, typename TEnable = void>
struct convert_base_types_helper;
}  // namespace convert_base_types_detail

/**
 * @brief This trait converts any non-struct/non-class types between two
 * different ABIs
 *
 * @tparam T is the type to be converted
 * @tparam abi_template_decls is the target ABI
 * @tparam TEnable is the dummy template parameter that allows us to specialize
 * the implementation for different types
 */
template <typename T, abi_template_decls>
using convert_base_types_t =
    typename convert_base_types_detail::convert_base_types_helper<
        T, abi_template_names>::type;

namespace convert_base_types_detail {

/**
 * @brief This specialization ensures types like `void`, `bool` etc. are not
 * changed by the ABI convertor
 */
template <typename T, abi_template_decls>
struct convert_base_types_helper<
    T, abi_template_names,
    std::enable_if_t<
        std::is_same_v<void, T> || std::is_same_v<std::nullptr_t, T> ||
        std::is_same_v<bool, T> || std::is_same_v<char, T> ||
        std::is_same_v<signed char, T> || std::is_same_v<char16_t, T> ||
        std::is_same_v<char32_t, T> ||
#if __cplusplus >= 202002L
        std::is_same_v<char8_t, T> ||
#endif
        std::is_floating_point_v<T> || std::is_enum_v<T>>> {
  using type = T;
};

/**
 * @brief This specialization converts `wchar_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<wchar_t, abi_template_names> {
  using type = TWCharType;
};

/**
 * @brief This specialization converts `short`
 */
template <abi_template_decls>
struct convert_base_types_helper<short, abi_template_names> {
  using type = TShortType;
};

/**
 * @brief This specialization converts `int`
 */
template <abi_template_decls>
struct convert_base_types_helper<int, abi_template_names> {
  using type = TIntType;
};

/**
 * @brief This specialization converts `long`
 */
template <abi_template_decls>
struct convert_base_types_helper<long, abi_template_names> {
  using type = TLongType;
};

/**
 * @brief This specialization converts `long long`
 */
template <abi_template_decls>
struct convert_base_types_helper<long long, abi_template_names> {
  using type = TLongLongType;
};

/**
 * @brief This specialization converts `unsigned T`
 */
template <typename T, abi_template_decls>
struct convert_base_types_helper<
    T, abi_template_names,
    std::enable_if_t<
        // Don't check with `is_unsigned_t` as this will permit qualifier and
        // cause instantiation confusion with other overloads
        std::is_same_v<T, unsigned char> || std::is_same_v<T, unsigned short> ||
        std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long> ||
        std::is_same_v<T, unsigned long long>>> {
  using type = std::make_unsigned_t<
      convert_base_types_t<std::make_signed_t<T>, abi_template_names>>;
};

/**
 * @brief This specialization converts `rlbox_uint8_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_uint8_t, abi_template_names> {
  using type = uint8_t;
};

/**
 * @brief This specialization converts `rlbox_int8_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_int8_t, abi_template_names> {
  using type = int8_t;
};

/**
 * @brief This specialization converts `rlbox_uint16_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_uint16_t, abi_template_names> {
  using type = uint16_t;
};

/**
 * @brief This specialization converts `rlbox_int16_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_int16_t, abi_template_names> {
  using type = int16_t;
};

/**
 * @brief This specialization converts `rlbox_uint32_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_uint32_t, abi_template_names> {
  using type = uint32_t;
};

/**
 * @brief This specialization converts `rlbox_int32_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_int32_t, abi_template_names> {
  using type = int32_t;
};

/**
 * @brief This specialization converts `rlbox_uint64_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_uint64_t, abi_template_names> {
  using type = uint64_t;
};

/**
 * @brief This specialization converts `rlbox_int64_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_int64_t, abi_template_names> {
  using type = int64_t;
};

/**
 * @brief This specialization converts `rlbox_size_t`
 */
template <abi_template_decls>
struct convert_base_types_helper<rlbox_size_t, abi_template_names> {
  using type = TSizeType;
};

/**
 * @brief This specialization converts `T&`, `T&&`, `const T`, `volatile T`
 */
template <typename T, abi_template_decls>
struct convert_base_types_helper<T, abi_template_names,
                                 std::enable_if_t<detail::is_cvref_t<T>>> {
  using type = detail::copy_cvref_t<
      T, convert_base_types_t<detail::remove_cvref_t<T>, abi_template_names>>;
};

/**
 * @brief This specialization converts `T[N]`
 */
template <typename T, abi_template_decls, size_t TN>
struct convert_base_types_helper<T[TN], abi_template_names,
                                 std::enable_if_t<!detail::is_cvref_t<T[TN]>>> {
  using array_element_type = convert_base_types_t<T, abi_template_names>;
  using type = array_element_type[TN];
};

/**
 * @brief This specialization converts `T*` and is behaves differently if the
 * target type of conversion is a `void*` or other types
 * @details This implements the following algorithm
 * ```
 * if std::is_pointer_v<TPointerType>
 *   TResult = copy_cvref_t(T, remove_pointer_t<TPointerType>) *
 * else
 *   TResult = TPointerType
 * ```
 *
 * This means that if we convert pointers to a type like void*, we want to
 * preserve cvref qualifiers of the pointer **basetype** during this
 * conversion.
 *
 * Eg: `const short *` is converted to `const void *`
 *
 * However, if we convert pointers to integer or struct types, then we lose
 * these qualifers of the pointer **basetype**.
 *
 * Eg: `const short *` is converted to `int`
 *
 * The qualifiers of the pointer type itself are always preserved.
 *
 * Eg: `short * const` is converted to `const int`
 */
template <typename T, abi_template_decls>
struct convert_base_types_helper<T*, abi_template_names,
                                 std::enable_if_t<!detail::is_cvref_t<T*>>> {
  using type = std::conditional_t<
      std::is_pointer_v<TPointerType>,
      detail::copy_cvref_t<T, std::remove_pointer_t<TPointerType>>*,
      TPointerType>;
};
}  // namespace convert_base_types_detail

}  // namespace rlbox::detail

#undef abi_template_decls
#undef abi_template_names
