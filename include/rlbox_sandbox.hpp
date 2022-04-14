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

private:
  template<typename T>
  using TDefaultTainted = tainted_relocatable<T, TSbx>;

public:
  template<typename T>
  using tainted =
    detail::get_member_tainted_type_or_default_t<TSbx, TDefaultTainted, T>;
};

}