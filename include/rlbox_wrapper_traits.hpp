#pragma once

/**
 * @file rlbox_wrapper_traits.hpp
 * @brief This header contains misc utilities to reflect on rlbox specific types
 */

#include <type_traits>

namespace rlbox::detail {

namespace detail_get_typemember_tainted_or_default {
  template<class TSbx,
           template<typename>
           class TDefaultTainted,
           class TEnable = void>
  struct get_typemember_tainted_or_default : std::false_type
  {
    template<typename T>
    using type = TDefaultTainted<T>;
  };

  template<class TSbx, template<typename> class TDefaultTainted>
  struct get_typemember_tainted_or_default<
    TSbx,
    TDefaultTainted,
    std::void_t<typename TSbx::template tainted<int>>> : std::true_type
  {
    template<typename T>
    using type = typename TSbx::template tainted<T>;
  };
}

/**
 * @brief Given class TSbx, get member "typename TSbx::tainted<T>" if it exists
 * else return TDefaultTainted<T>
 * This is based on
 * https://stackoverflow.com/questions/9644477/how-to-check-whether-a-class-has-specified-nested-class-definition-or-typedef-in

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
 *   using tainted = Custom<T>;
 * };
 *
 * get_typemember_tainted_or_default_t<foo, Default, int> == Custom<int>
 *
 * @tparam TSbx Sandbox plugin type
 * @tparam TDefaultTainted the default tainted implementation
 * @tparam T the underlying data type that is tainted
 */
template<class TSbx, template<typename> class TDefaultTainted, class T>
using get_typemember_tainted_or_default_t =
  typename detail_get_typemember_tainted_or_default::
    get_typemember_tainted_or_default<TSbx, TDefaultTainted>::template type<T>;

////////////////////////////////////////////////////////////////////////

}