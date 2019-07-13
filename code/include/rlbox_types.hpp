#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

namespace rlbox {

template<template<typename, typename> typename T_Wrap,
         typename T,
         typename T_Sbx>
class tainted_base_impl;

template<typename T, typename T_Sbx>
class tainted;

template<typename T, typename T_Sbx>
class tainted_volatile;

template<typename T_Sbx>
class RLBoxSandbox;

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

class tainted_marker
{};
class tainted_volatile_marker
{};

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