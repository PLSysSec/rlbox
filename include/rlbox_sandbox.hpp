#pragma once

#include "rlbox_tainted_relocatable.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

// The type that represents a sandbox. This type provides APIs to invoke
// functions in the sandboxed component, expose callbacks, retrieve informtion
// about the sandbox memory etc. TSbx is the type of the sandbox plugin that
// represents the underlying sandbox implementation.
template<typename TSbx>
class rlbox_sandbox
{
public:
  template<typename T>
  using tainted =
    detail::get_member_tainted_or_default_t<TSbx, tainted_relocatable<T, TSbx>>;
};

}