/**
 * @file rlbox_wrapper_traits.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc utilities to reflect on rlbox specific types
 */

#pragma once

#include "rlbox_abi_conversion.hpp"
#include "rlbox_tainted_base.hpp"
// IWYU incorrectly reports this as unnecessary as the use of rlbox_type_traits
// is in a templated class
#include "rlbox_type_traits.hpp"  // IWYU pragma: keep

namespace rlbox::detail {

/**
 * @brief RLBox converts data between the host ABI and the sandbox ABI as they
 * may be different. The sandbox ABI details is defined in the sandbox plugin.
 * This trait takes these details and constructs a type of @ref
 * rlbox::detail::convert_base_types_t which is capable of converting primtive
 * types to the target sandbox ABI.
 * @tparam T is the type to convert to the sandbox ABI
 * @tparam TSbx is the sandbox plugin to whose ABI the type is being converted
 */
template <typename T, typename TSbx>
using rlbox_base_types_convertor =
    convert_base_types_t<T, typename TSbx::sbx_wchart, typename TSbx::sbx_short,
                         typename TSbx::sbx_int, typename TSbx::sbx_long,
                         typename TSbx::sbx_longlong, typename TSbx::sbx_sizet,
                         typename TSbx::sbx_pointer>;

/**
 * @brief This trait identifies if a given generic types is a tainted wrapper.
 * It does this by checking if the generic wrapper derives from @ref
 * rlbox::tainted_base
 * @tparam TWrap is the generic type to check
 * @tparam T is the type of the data being wrapped over
 * @tparam TSbx is the sandbox type
 */
template <template <typename, typename...> typename TWrap, typename T,
          typename TSbx>
constexpr bool is_tainted_wrapper =
    std::is_base_of_v<tainted_base<T, TSbx>, TWrap<T, TSbx>>;

}  // namespace rlbox::detail