/**
 * @file rlbox_function_traits.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc function traits useful for rlbox
 */

#pragma once

namespace rlbox::detail {

// namespace convert_to_cfunc_detail {
// /**
//  * @brief This trait is the internal helper to implement convert_to_cfunc_t
//  * @tparam T is the C++ function type
//  */
// template <typename T>
// struct helper;

// template <typename TRet, typename... TArgs>
// struct helper<TRet(TArgs...)> {
//   using type = TRet(TArgs...);
// };

// template <typename TRet, typename TClass, typename... TArgs>
// struct helper<TRet (TClass::*)(TArgs...)> {
//   using type = TRet(TClass*, TArgs...);
// };

// template <typename TRet, typename TClass, typename... TArgs>
// struct helper<TRet (TClass::*)(TArgs...) const> {
//   using type = TRet(TClass*, TArgs...);
// };
// }  // namespace convert_to_cfunc_detail

// /**
//  * @brief This trait converts the C++ function type to a C function type. If
//  the
//  * argument is a C style function, this is unchanged. If the argument is C++
//  * class member function, this function prepends a pointer to the class as
//  the
//  * first parameter.
//  * @tparam TFunc is type of the function being converted
//  */
// template <typename TFunc>
// using convert_to_cfunc_t =
//     typename convert_to_cfunc_detail::helper<TFunc>::type;

///////////////////////////////////////////////////////////////////////////////

namespace prepend_func_arg_detail {
/**
 * @brief This trait is the internal helper to implement prepend_func_arg_t
 * @tparam TFunc is the function type
 * @tparam TArgNew is the argument type to prepend as the first argument
 */
template <typename TFunc, typename TArgNew>
struct helper;

template <typename TArgNew, typename TRet, typename... TArgs>
struct helper<TRet(TArgs...), TArgNew> {
  using type = TRet(TArgNew, TArgs...);
};

template <typename TArgNew, typename TRet, typename TClass, typename... TArgs>
struct helper<TRet (TClass::*)(TArgs...), TArgNew> {
  using type = TRet (TClass::*)(TArgNew, TArgs...);
};

template <typename TArgNew, typename TRet, typename TClass, typename... TArgs>
struct helper<TRet (TClass::*)(TArgs...) const, TArgNew> {
  using type = TRet (TClass::*)(TArgNew, TArgs...) const;
};
}  // namespace prepend_func_arg_detail

/**
 * @brief This trait allows modifiying a function type by pre-pending a new
 * first argument
 * @tparam TFunc is the function type
 * @tparam TArgNew is the argument type to prepend as the first argument
 */
template <typename TFunc, typename TArgNew>
using prepend_func_arg_t =
    typename prepend_func_arg_detail::helper<TFunc, TArgNew>::type;

///////////////////////////////////////////////////////////////////////////////

namespace func_type_converter_detail {
/**
 * @brief This trait is the internal helper to implement func_type_converter_t
 * @tparam TFunc is the function type
 * @tparam TConv is the convertor which takes an argument of the type to
 * converted and returns the converted type
 */
template <typename TFunc, template <typename T> class TConv>
struct helper;

template <template <typename T> class TConv, typename TRet, typename... TArgs>
struct helper<TRet(TArgs...), TConv> {
  using type = TConv<TRet>(TConv<TArgs>...);
};

template <template <typename T> class TConv, typename TRet, typename TClass,
          typename... TArgs>
struct helper<TRet (TClass::*)(TArgs...), TConv> {
  using type = TConv<TRet> (TConv<TClass>::*)(TConv<TArgs>...);
};

template <template <typename T> class TConv, typename TRet, typename TClass,
          typename... TArgs>
struct helper<TRet (TClass::*)(TArgs...) const, TConv> {
  using type = TConv<TRet> (TConv<TClass>::*)(TConv<TArgs>...) const;
};
}  // namespace func_type_converter_detail

/**
 * @brief This trait applies a provided type convertor to all parameter types
 * and the return type of a function
 * @tparam TFunc is the function type
 * @tparam TConv is the convertor which takes an argument of the type to
 * converted and returns the converted type
 */
template <typename TFunc, template <typename T> class TConv>
using func_type_converter_t =
    typename func_type_converter_detail::helper<TFunc, TConv>::type;

}  // namespace rlbox::detail
