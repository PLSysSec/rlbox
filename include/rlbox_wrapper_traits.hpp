/**
 * @file rlbox_wrapper_traits.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc utilities to reflect on rlbox specific types
 */

#pragma once

#include <stddef.h>
#include <type_traits>

#include "rlbox_helpers.hpp"
#include "rlbox_stdint_types.hpp"
#include "rlbox_tainted_base.hpp"
// IWYU incorrectly reports this as unnecessary as the use of rlbox_type_traits
// is in a templated class
#include "rlbox_type_traits.hpp"  // IWYU pragma: keep

namespace rlbox::detail {

/**
 * @brief This trait checks to see if the sandbox uses a different ABI from that
 * of the host application.
 * @tparam TSbx is the sandbox type for which we are checking the ABI.
 * @details The ABI is "different" if any of the base types are different from
 * the host and the sandbox code. Pointer are handled a little differently. The
 * ABI is considered "identical" if pointers are represented by an integer or
 * pointer type and has the same size as a pointer in the host.
 */
template <typename TSbx>
constexpr bool rlbox_base_types_unchanged_v =
    std::is_same_v<typename TSbx::sbx_wchart, wchar_t>&&
        std::is_same_v<typename TSbx::sbx_short, short>&&
            std::is_same_v<typename TSbx::sbx_int, int>&&
                std::is_same_v<typename TSbx::sbx_long, long>&&
                    std::is_same_v<typename TSbx::sbx_longlong, long long>&&
                        std::is_same_v<typename TSbx::sbx_sizet, size_t> &&
    // Pointers could be represented with either integer or pointer type and
    // have the same ABI
    sizeof(typename TSbx::sbx_pointer) == sizeof(void*) &&
    (std::is_integral_v<typename TSbx::sbx_pointer> ||
     std::is_pointer_v<typename TSbx::sbx_pointer>);

/**
 * @brief This trait checks to see if the sandbox uses an ABI not larger from
 * that of the host application, i.e., each type in the sandbox is the same or
 * smaller size than that of the host.
 * @tparam TSbx is the sandbox type for which we are checking the ABI.
 */
template <typename TSbx>
constexpr bool rlbox_base_types_not_larger_v =
    sizeof(typename TSbx::sbx_wchart) <= sizeof(wchar_t) &&
    sizeof(typename TSbx::sbx_short) <= sizeof(short) &&
    sizeof(typename TSbx::sbx_int) <= sizeof(int) &&
    sizeof(typename TSbx::sbx_long) <= sizeof(long) &&
    sizeof(typename TSbx::sbx_longlong) <= sizeof(long long) &&
    sizeof(typename TSbx::sbx_sizet) <= sizeof(size_t) &&
    sizeof(typename TSbx::sbx_pointer) <= sizeof(void*);

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
constexpr bool is_tainted_wrapper_v =
    std::is_base_of_v<tainted_base<T, TSbx>, TWrap<T, TSbx>>;

/**
 * @brief This trait identifies if a given type is an RLBox stdint type (See
 * @ref rlbox_stdint_types.hpp). It does this by checking if the type derives
 * from @ref rlbox::detail::rlbox_stdint_base_t
 * @tparam T is the type to check
 */
template <typename T>
constexpr bool is_rlbox_stdint_type_v =
    std::is_base_of_v<rlbox_stdint_base_t, T>;

/**
 * @brief Macro that generates a trait to check if member functions named
 * `membername<void*>` exist for the given class `T`.
 */
#define rlbox_detail_has_template_member(membername)            \
  namespace detail_has_member {                                 \
  template <typename T, typename = std::void_t<>>               \
  struct has_member_##membername : std::false_type {};          \
                                                                \
  template <typename T>                                         \
  struct has_member_##membername<                               \
      T, std::void_t<decltype(&T::template membername<void*>)>> \
      : std::true_type {};                                      \
  }                                                             \
                                                                \
  template <typename T>                                         \
  inline constexpr bool has_member_##membername##_v =           \
      detail_has_member::has_member_##membername<T>::value;     \
  RLBOX_REQUIRE_SEMI_COLON

/**
 * @brief Create trait to check for member `TSbx::impl_malloc_in_sandbox<void*>`
 */
rlbox_detail_has_template_member(impl_malloc_in_sandbox);
/**
 * @brief Create trait to check for member `TSbx::impl_free_in_sandbox<void*>`
 */
rlbox_detail_has_template_member(impl_free_in_sandbox);

}  // namespace rlbox::detail