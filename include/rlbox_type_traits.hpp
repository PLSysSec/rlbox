/**
 * @file rlbox_type_traits.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc type traits useful for rlbox
 */

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "rlbox_error_handling.hpp"
#include "rlbox_helpers.hpp"

namespace rlbox::detail {

/**
 * @brief An expression that always evaluates to false. Useful for
 * static_assert(false) that is evaluated inside a constexpr.
 * @tparam T is a template parameter to ensure that the expression is evaluated
 * lazily
 */
template <typename T>
inline constexpr bool false_v =
    rlbox::detail::fail_detail::helper_struct<T>::mVal;

namespace copy_cvref_detail {
/**
 * @brief This trait is the internal helper to implement copy_cvref_t
 * @tparam TSource is the type from which we copy the cvref qualifiers
 * @tparam TDest is the type to which we copy the cvref qualifiers
 */
template <typename TSource, typename TDest>
struct copy_cvref {
  // Implementation adapted from https://stackoverflow.com/a/31173086
 private:
  using R = std::remove_reference_t<TSource>;
  using U1 =
      std::conditional_t<std::is_const_v<R>, std::add_const_t<TDest>, TDest>;
  using U2 =
      std::conditional_t<std::is_volatile_v<R>, std::add_volatile_t<U1>, U1>;
  using U3 = std::conditional_t<std::is_lvalue_reference_v<TSource>,
                                std::add_lvalue_reference_t<U2>, U2>;
  using U4 = std::conditional_t<std::is_rvalue_reference_v<TSource>,
                                std::add_rvalue_reference_t<U3>, U3>;

 public:
  using type = U4;
};
}  // namespace copy_cvref_detail

/**
 * @brief Copies the cvref qualifiers from `TSource` to `TDest`.
 * @code {.cpp}
 * static_assert(std::is_same_t<copy_cvref_t<const int&, short>, const short&>);
 * @endcode
 * @tparam TSource is the type from which we copy the cvref qualifiers
 * @tparam TDest is the type to which we copy the cvref qualifiers
 */
template <typename TSource, typename TDest>
using copy_cvref_t =
    typename copy_cvref_detail::copy_cvref<TSource, TDest>::type;

/**
 * @brief Polyfill for C++20's std::remove_cvref_t
 * @tparam T is the type for which we are removing cvref qualifiers
 */
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

/**
 * @brief A trait that returns true if the type `T` has any cvref qualifiers
 * @tparam T is the type for which is checked for cvref qualifiers
 */
template <typename T>
inline constexpr bool is_cvref_t = !std::is_same_v<remove_cvref_t<T>, T>;

///////////////////////////////////////////////////////////////////////////////

namespace array_detail {
/**
 * @brief This handles the case where T is not an array
 * @tparam T is the type to convert
 */
template <typename T>
struct array_detail_helper {
  using c_array_to_std_array_t = T;
  using std_array_to_c_array_t = T;
  static constexpr bool mIsArray = false;
};

/**
 * @brief This handles the case where T is an array of size N
 * @tparam T is the type to convert
 */
template <typename T, size_t TN>
struct array_detail_helper<T[TN]> {
  using c_array_to_std_array_t = std::array<T, TN>;
  using std_array_to_c_array_t = T[TN];
  static constexpr bool mIsArray = true;
};

/**
 * @brief This handles the case where the argument is a std::array of type T
 * and size N
 * @tparam T is the type to convert
 */
template <typename T, size_t TN>
struct array_detail_helper<std::array<T, TN>> {
  using c_array_to_std_array_t = std::array<T, TN>;
  using std_array_to_c_array_t = T[TN];
  static constexpr bool mIsArray = true;
};

/**
 * @brief This handles the case for dynamic sized arrays (like `int[]') which
 * are not supported.
 * @tparam T is the type to convert
 */
template <typename T>
struct array_detail_helper<T[]> {
  using c_array_to_std_array_t = T[];
  using std_array_to_c_array_t = T[];
  static constexpr bool mIsArray = true;
};

}  // namespace array_detail

/**
 * @brief If T is a `int[N]', this trait converts it to the `std::array<N>'
 * type for this, else T is returned. Dynamic sized arrays `int[]' are not
 * supported
 * @tparam T is the type to convert
 */
template <typename T>
using c_array_to_std_array_t =
    typename array_detail::array_detail_helper<T>::c_array_to_std_array_t;

/**
 * @brief If T is a `std::array<N>', this trait converts it to the `int[N]'
 * type for this, else T is returned. Dynamic sized arrays `int[]' are not
 * supported
 * @tparam T is the type to convert
 */
template <typename T>
using std_array_to_c_array_t =
    typename array_detail::array_detail_helper<T>::std_array_to_c_array_t;

/**
 * @brief If T is a `T[N]' or `std::array<T, N>'
 * @tparam T is the type to check
 */
template <typename T>
constexpr bool is_any_array_v = array_detail::array_detail_helper<T>::mIsArray;

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Returns the value-type equivalent of the given type. Currently this is
 * only needed to convert c style arrays to std::arrays
 * @tparam T is the type to convert
 */
template <typename T>
using value_type_t = c_array_to_std_array_t<T>;

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is just `is_fundamental_v<T> || std::is_enum_v<T>`
 * @tparam T is the type to check
 */
template <typename T>
constexpr bool is_fundamental_or_enum_v =
    std::is_fundamental_v<T> || std::is_enum_v<T>;

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is just `is_fundamental_v<T> || std::is_enum_v<T> ||
 * std::is_pointer_v<T>`
 * @tparam T is the type to check
 */
template <typename T>
constexpr bool is_fundamental_or_enum_or_pointer_v =
    std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

///////////////////////////////////////////////////////////////////////////////

namespace get_equivalent_uint_detail {

template <typename T, typename TDummy = void>
struct helper {
  static_assert(false_v<T>, "Unsupported pointer size");
};

template <typename T>
struct helper<T, RLBOX_SPECIALIZE(sizeof(T) == 1)> {
  using type = uint8_t;
};

template <typename T>
struct helper<T, RLBOX_SPECIALIZE(sizeof(T) == 2)> {
  using type = uint16_t;
};

template <typename T>
struct helper<T, RLBOX_SPECIALIZE(sizeof(T) == 4)> {
  using type = uint32_t;
};

template <typename T>
struct helper<T, RLBOX_SPECIALIZE(sizeof(T) == 8)> {
  using type = uint64_t;
};

};  // namespace get_equivalent_uint_detail

/**
 * @brief Get the integer size that corresponds to the size of argument
 * @tparam T is the argument whose size is checked
 */
template <typename T>
using get_equivalent_uint_t =
    typename get_equivalent_uint_detail::helper<T>::type;

///////////////////////////////////////////////////////////////////////////////

namespace apply_conv_to_base_types_detail {

template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers, typename TEnable = void>
struct helper;

/**
 * @brief This specialization converts `T*` when pointers are treated as
 * compound types, i.e. we generate TConv<T>*
 */
template <typename T, template <typename> typename TConv>
struct helper<T*, TConv, false, RLBOX_SPECIALIZE(!detail::is_cvref_t<T*>)> {
  using ptr_type = typename helper<T, TConv, false>::type;
  using type = std::add_pointer_t<ptr_type>;
};

/**
 * @brief This specialization converts `T*` when pointers are treated as
 * base types, i.e. we generate TConv<T*>
 */
template <typename T, template <typename> typename TConv>
struct helper<T*, TConv, true, RLBOX_SPECIALIZE(!detail::is_cvref_t<T*>)> {
  using type = TConv<T*>;
};

/**
 * @brief This specialization converts `std::array<T, TN>`
 */
template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers, size_t TN>
struct helper<std::array<T, TN>, TConv, TCallTConvOnPointers,
              RLBOX_SPECIALIZE(!detail::is_cvref_t<std::array<T, TN>>)> {
  using array_element_type =
      typename helper<T, TConv, TCallTConvOnPointers>::type;
  using type = std::array<array_element_type, TN>;
};

/**
 * @brief This specialization converts `T[N]`
 */
template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers, size_t TN>
struct helper<T[TN], TConv, TCallTConvOnPointers,
              RLBOX_SPECIALIZE(!detail::is_cvref_t<T[TN]>)> {
  using array_element_type =
      typename helper<T, TConv, TCallTConvOnPointers>::type;
  using type = array_element_type[TN];
};

/**
 * @brief This specialization converts `T[]`
 */
template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers>
struct helper<T[], TConv, TCallTConvOnPointers,
              RLBOX_SPECIALIZE(!detail::is_cvref_t<T[]>)> {
  using array_element_type =
      typename helper<T, TConv, TCallTConvOnPointers>::type;
  using type = array_element_type[];
};

/**
 * @brief This specialization converts structs, enums etc.
 */
template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers>
struct helper<T, TConv, TCallTConvOnPointers,
              RLBOX_SPECIALIZE(!std::is_pointer_v<T> &&
                               !detail::is_any_array_v<T> &&
                               !detail::is_cvref_t<T>)> {
  using type = TConv<T>;
};

/**
 * @brief This specialization converts `T&`, `T&&`, `const T`, `volatile T`
 */
template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers>
struct helper<T, TConv, TCallTConvOnPointers,
              RLBOX_SPECIALIZE(detail::is_cvref_t<T>)> {
  using inner_type = typename helper<detail::remove_cvref_t<T>, TConv,
                                     TCallTConvOnPointers>::type;
  using type = detail::copy_cvref_t<T, inner_type>;
};

};  // namespace apply_conv_to_base_types_detail

template <typename T, template <typename> typename TConv,
          bool TCallTConvOnPointers = false>
using apply_conv_to_base_types =
    typename apply_conv_to_base_types_detail::helper<
        T, TConv, TCallTConvOnPointers>::type;

}  // namespace rlbox::detail