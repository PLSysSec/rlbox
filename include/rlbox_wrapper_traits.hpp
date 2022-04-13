#pragma once

// This header contains misc utilities to reflect on rlbox specific types

#include <type_traits>

namespace rlbox::detail {

// Given a class T, get the member typedef T::tainted if it exists
// else return T_Default

// https://stackoverflow.com/questions/9644477/how-to-check-whether-a-class-has-specified-nested-class-definition-or-typedef-in
namespace detail_get_member_tainted_or_default {
  template<class T, class TDefault, class TEnable = void>
  struct get_member_tainted_or_default
  {
    using type = TDefault;
  };

  template<class T>
  struct get_member_tainted_or_default<T, std::void_t<typename T::tainted>>
  {
    using type = typename T::tainted;
  };
}

template<class T, class TDefault>
using get_member_tainted_or_default_t =
  typename detail_get_member_tainted_or_default::
    get_member_tainted_or_default<T, TDefault>::type;

////////////////////////////////////////////////////////////////////////

}