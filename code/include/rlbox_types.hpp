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

}