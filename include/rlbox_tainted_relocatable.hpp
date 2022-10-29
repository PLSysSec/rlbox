/**
 * @file rlbox_tainted_relocatable.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_relocatable class.
 */

#pragma once

#include <stdlib.h>
// IWYU incorrectly reports this as unnecessary as the use of type_traits is in
// a templated class
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_type_traits.hpp"

namespace rlbox {

/**
 * @brief Specialization for non-fundamental types
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
class tainted_relocatable_helper : public tainted_base<T, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 private:
  using TSbxRep = detail::rlbox_base_types_convertor<T, TSbx>;

  // NOLINTNEXTLINE(hicpp-use-nullptr, modernize-use-nullptr)
  detail::value_type_t<TSbxRep> data{0};

 public:
  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @return detail::value_type_t<T> is the raw data
   */
  [[nodiscard]] inline detail::value_type_t<T> UNSAFE_unverified() const {
    return abort();
  }

  [[nodiscard]] inline detail::value_type_t<TSbxRep> UNSAFE_sandboxed() const {
    return abort();
  }
};

/**
 * @brief Implementation of tainted data wrappers that supports the movement of
 * the sandbox heap after creation.
 *
 * @note This implementatation handles only non-class/non-struct and non-pointer
 * T types and its behavior is identical to tainted_fixed_aligned.
 * Specializations of this class are generated by the rlbox_lib_load_classes
 * when T is a class/struct.
 *
 * @tparam T is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <typename T, typename TSbx>
using tainted_relocatable =
    std::conditional_t<detail::is_fundamental_or_enum_v<T>,
                       tainted_fixed_aligned<T, TSbx>,
                       tainted_relocatable_helper<T, TSbx>>;

}  // namespace rlbox