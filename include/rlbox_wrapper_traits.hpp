#pragma once

/**
 * @file rlbox_wrapper_traits.hpp
 * @brief This header contains misc utilities to reflect on rlbox specific types
 */

#include <type_traits>

namespace rlbox::detail {

/**
 * @brief Given class TSbx, get member "membername<T>" if it exists
 * else return TDefaultTainted<T>
 * This is based on
 * https://stackoverflow.com/questions/9644477/how-to-check-whether-a-class-has-specified-nested-class-definition-or-typedef-in
 *
 * @code
 * template<typename T>
 * struct Default {};
 *
 * template<typename T>
 * struct Custom {};
 *
 * class foo {
 * public:
 *   template<typename T>
 *   using membername = Custom<T>;
 * };
 *
 * get_typemember_membername_or_default_t<foo, Default, int> == Custom<int>
 *
 * @tparam TSbx Sandbox plugin type
 * @tparam TDefaultTainted the default membername implementation
 * @tparam T the underlying data type that is membername
 */
#define detail_get_typemember_membername_or_default(membername)                \
  namespace detail_get_typemember_##membername##_or_default                    \
  {                                                                            \
    template<class TSbx,                                                       \
             template<typename>                                                \
             class TDefaultTainted,                                            \
             class TEnable = void>                                             \
    struct get_typemember_##membername##_or_default : std::false_type          \
    {                                                                          \
      template<typename T>                                                     \
      using type = TDefaultTainted<T>;                                         \
    };                                                                         \
                                                                               \
    template<class TSbx, template<typename> class TDefaultTainted>             \
    struct get_typemember_##membername##_or_default<                           \
      TSbx,                                                                    \
      TDefaultTainted,                                                         \
      std::void_t<typename TSbx::template membername<int>>> : std::true_type   \
    {                                                                          \
      template<typename T>                                                     \
      using type = typename TSbx::template membername<T>;                      \
    };                                                                         \
  }

detail_get_typemember_membername_or_default(tainted);
detail_get_typemember_membername_or_default(tainted_volatile);

/**
 * @brief Given class TSbx, get member "typename TSbx::tainted<T>" if it exists
 * else return TDefaultTainted<T>
 *
 * @tparam TSbx Sandbox plugin type
 * @tparam TDefaultTainted the default tainted implementation
 * @tparam T the underlying data type that is tainted
 */
template<class TSbx, template<typename> class TDefaultTainted, class T>
using get_typemember_tainted_or_default_t =
  typename detail_get_typemember_tainted_or_default::
    get_typemember_tainted_or_default<TSbx, TDefaultTainted>::template type<T>;

/**
 * @brief Given class TSbx, get member "typename TSbx::tainted_volatile<T>" if
 * it exists else return TDefaultTainted<T>
 *
 * @tparam TSbx Sandbox plugin type
 * @tparam TDefaultTainted the default tainted_volatile implementation
 * @tparam T the underlying data type that is tainted_volatile
 */
template<class TSbx, template<typename> class TDefaultTainted, class T>
using get_typemember_tainted_volatile_or_default_t =
  typename detail_get_typemember_tainted_volatile_or_default::
    get_typemember_tainted_volatile_or_default<TSbx, TDefaultTainted>::
      template type<T>;

////////////////////////////////////////////////////////////////////////

}