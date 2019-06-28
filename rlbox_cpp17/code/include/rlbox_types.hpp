#pragma once

namespace rlbox {

template<typename T, typename T_Sbx>
class tainted;

template<typename T, typename T_Sbx>
class tainted_volatile;

/* Trait types */

class sandbox_wrapper_base
{};

template<typename T>
class sandbox_wrapper_base_of
{};

template<typename T, typename T_Sbx>
class tainted_base
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T>
{};

// Check to ensure we don't accidentally introduce vtables which would slow
// things down
static_assert(sizeof(sandbox_wrapper_base) < sizeof(uintptr_t));
static_assert(sizeof(sandbox_wrapper_base_of<int>) < sizeof(uintptr_t));
static_assert(sizeof(tainted_base<int, int>) < sizeof(uintptr_t));

}