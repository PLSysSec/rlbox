/**
 * @file rlbox_abi_conversion.hpp
 * @brief The file provides utilities to convert between different ABIs
 * @details In particular we can convert data structures, between 2 ABIs the
 * source ABI to a target ABI. Each ABI can specify custom sizes of short,
 * int, long, and pointer sizes. Thus the type `int` may have different sizes in
 * the source and target ABI. This file provides a trait allows conversions
 * of these types from the source to the target ABI.
 */
#pragma once

#include <stddef.h>
#include <type_traits>

#include "rlbox_type_traits.hpp"

/**
 * @brief abi_template_decls is the list of types that can be converted by this
 * ABI convertor
 */
#define abi_template_decls                                                     \
  typename TWCharType, typename TShortType, typename TIntType,                 \
    typename TLongType, typename TLongLongType, typename TPointerType

/**
 * @brief abi_template_names is the list of typenames in abi_template_decls
 */
#define abi_template_names                                                     \
  TWCharType, TShortType, TIntType, TLongType, TLongLongType, TPointerType

namespace rlbox::detail {

namespace convert_detail {

  /**
   * @brief This trait is the internal helper to implement convert_base_types_t
   *
   * @tparam T is the type to be converted
   * @tparam abi_template_decls is the target ABI
   * @tparam TEnable is the dummy template paramter that allows us to specialize
   * the implementation for different types
   */
  template<typename T, abi_template_decls, typename TEnable = void>
  struct convert_base_types_t_helper;
}

/**
 * @brief This trait converts any non-struct/non-class types between two
 * different ABIs
 *
 * @tparam T is the type to be converted
 * @tparam abi_template_decls is the target ABI
 * @tparam TEnable is the dummy template paramter that allows us to specialize
 * the implementation for different types
 */
template<typename T, abi_template_decls>
using convert_base_types_t = typename convert_detail::
  convert_base_types_t_helper<T, abi_template_names>::type;

namespace convert_detail {

  /**
   * @brief This specialization ensures types like `void`, `bool` etc. are not
   * changed by the ABI convertor
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<
    T,
    abi_template_names,
    std::enable_if_t<
      std::is_same_v<void, T> || std::is_same_v<bool, T> ||
      std::is_same_v<char, T> || std::is_same_v<signed char, T> ||
      std::is_same_v<char16_t, T> || std::is_same_v<char32_t, T> ||
#if __cplusplus >= 202002L
      std::is_same_v<char32_t, T>
#endif
        std::is_floating_point_v<T> ||
      std::is_enum_v<T>>>
  {
    using type = T;
  };

  /**
   * @brief This specialization converts `wchar_t`
   */
  template<abi_template_decls>
  struct convert_base_types_t_helper<wchar_t, abi_template_names>
  {
    using type = TWCharType;
  };

  /**
   * @brief This specialization converts `short`
   */
  template<abi_template_decls>
  struct convert_base_types_t_helper<short, abi_template_names>
  {
    using type = TShortType;
  };

  /**
   * @brief This specialization converts `int`
   */
  template<abi_template_decls>
  struct convert_base_types_t_helper<int, abi_template_names>
  {
    using type = TIntType;
  };

  /**
   * @brief This specialization converts `long`
   */
  template<abi_template_decls>
  struct convert_base_types_t_helper<long, abi_template_names>
  {
    using type = TLongType;
  };

  /**
   * @brief This specialization converts `long long`
   */
  template<abi_template_decls>
  struct convert_base_types_t_helper<long long, abi_template_names>
  {
    using type = TLongLongType;
  };

  /**
   * @brief This specialization converts `unsigned T`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<
    T,
    abi_template_names,
    std::enable_if_t<
      std::is_same_v<T, unsigned char> || std::is_same_v<T, unsigned short> ||
      std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long> ||
      std::is_same_v<T, unsigned long long>>>
  {
    using type = std::make_unsigned_t<
      convert_base_types_t<std::make_signed_t<T>, abi_template_names>>;
  };

  /**
   * @brief This specialization converts `T*`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<T*, abi_template_names>
  {
    // if TPointerType == pointer_type
    //    TResultBaseType = add_cv_ref_to(get_cv_ref(T),
    //    remove_ptr_t<TPointerType>) TResult = add_ptr_t<TResultBaseType>
    // else
    //    TResult = TPointerType
    // using type = std::conditional_t<std::is_pointer_v<TPointerType>,
    //                                 preserve_ptr_cv_ref_t<T*, TPointerType>,
    //                                 TPointerType>;
    using type = TPointerType;
  };

  /**
   * @brief This specialization converts `T&`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<T&, abi_template_names>
  {
    using type =
      std::add_lvalue_reference_t<convert_base_types_t<T, abi_template_names>>;
  };

  /**
   * @brief This specialization converts `T&&`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<T&&, abi_template_names>
  {
    using type =
      std::add_rvalue_reference_t<convert_base_types_t<T, abi_template_names>>;
  };

  /**
   * @brief This specialization converts `const T`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<const T,
                                     abi_template_names,
                                     std::enable_if_t<!std::is_reference_v<T>>>
  {
    using type = std::add_const_t<convert_base_types_t<T, abi_template_names>>;
  };

  /**
   * @brief This specialization converts `volatile T`
   */
  template<typename T, abi_template_decls>
  struct convert_base_types_t_helper<
    volatile T,
    abi_template_names,
    std::enable_if_t<!std::is_reference_v<T> && !std::is_const_v<T>>>
  {
    using type =
      std::add_volatile_t<convert_base_types_t<T, abi_template_names>>;
  };

  /**
   * @brief This specialization converts `T[N]`
   */
  template<typename T, abi_template_decls, size_t N>
  struct convert_base_types_t_helper<
    T[N],
    abi_template_names,
    std::enable_if_t<!std::is_reference_v<T> && !std::is_const_v<T> &&
                     !std::is_volatile_v<T>>>
  {
    using array_element_type = convert_base_types_t<T, abi_template_names>;
    using type = array_element_type[N];
  };
}

}

#undef abi_template_decls
#undef abi_template_names
