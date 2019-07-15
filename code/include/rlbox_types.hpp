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

// Use a custom enum for returns as boolean returns are a bad idea
// int returns are automatically cast to a boolean
// Some APIs have overloads with boolean and non returns, so best to use a
// custom class
enum class RLBox_Verify_Status
{
  SAFE,
  UNSAFE
};

}