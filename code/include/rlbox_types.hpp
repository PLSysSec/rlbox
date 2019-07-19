#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

namespace rlbox {

template<typename T, typename T_Sbx>
class tainted;

template<typename T, typename T_Sbx>
class tainted_volatile;

template<typename T_Sbx>
class RLBoxSandbox;

template<typename T, typename T_Sbx>
class sandbox_callback;

template<typename T, typename T_Sbx>
class sandbox_function;
}