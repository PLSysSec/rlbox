#pragma once

#include "rlbox_conversion.hpp"

namespace rlbox {

template<typename TSandbox>
class RLBoxSandbox;

template<typename T, typename TSandbox>
class tainted;

template<typename T, typename TSandbox>
class tainted_volatile;

/* Trait types */

class sandbox_wrapper_base
{};

template<typename T>
class sandbox_wrapper_base_of
{};

template<typename T, typename TSandbox>
class tainted_base
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T>
{};

////////////////

/*
Make sure tainted<T1> can access private members of tainted<T2>
Ideally, this should be

template <typename U1>
friend class tainted<U1, TSandbox>;

But C++ doesn't see to allow the above
*/
#define KEEP_THINGS_FRIENDLY                                                   \
  template<typename U1, typename U2>                                           \
  friend class tainted;                                                        \
                                                                               \
  template<typename U1, typename U2>                                           \
  friend class tainted_volatile;

template<typename T, typename TSandbox>
class tainted : public tainted_base<T, TSandbox>
{
  KEEP_THINGS_FRIENDLY

private:
  T field;
};

template<typename T, typename TSandbox>
class tainted_volatile : public tainted_base<T, TSandbox>
{
  KEEP_THINGS_FRIENDLY

private:
  T field;

public:
  tainted() = default;
  tainted(const tainted<T, TSandbox>& p) = default;

  tainted(const tainted_volatile<T, TSandbox>& p)
  {
    if constexpr (!std::is_array_v<T>) {
      field = p.UNSAFE_Unverified();
    } else {
      memcpy(field, p.field, sizeof(T));
    }
  }
};

#undef KEEP_THINGS_FRIENDLY
}