/**
 * @file rlbox_callback.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief The file provides the implementation of the rlbox_callback type, which
 * holds details of callbacks.
 */

#pragma once

#include "rlbox_error_handling.hpp"
#include "rlbox_types.hpp"

#include <type_traits>

namespace rlbox {

namespace callback_detail {

template <typename T, typename TSbx>
using tainted_or_void_t = std::conditional_t<std::is_void_v<T>, void, tainted<T, TSbx>>;

/**
 * @brief This trait computes the expected type of a modified/tainted
 * callback's function type, given the original function's parameters and
 * returns.
 *
 * @tparam TSbx is the sandbox type
 * @tparam TRet is the return type of the function
 * @tparam TArgs is the types of the function's arguments
 */
template <typename TSbx, typename TRet, typename... TArgs>
using external_callback_t =
    tainted_or_void_t<TRet, TSbx> (*)(rlbox_sandbox<TSbx>&, tainted<TArgs, TSbx>...);

template <typename TSbx, typename TRet, typename... TArgs>
external_callback_t<TSbx, TRet, TArgs...> external_callback_type_helper(
    TRet (*)(TArgs...));

};  // namespace callback_detail

/**
 * @brief A wrapper type used to mark any function pointers to application code
 * that is passed to the sandbox. This type indicates that the application has
 * explicitly allowed this function to be called by the sandbox.
 * @tparam TUseAppRep indicates whether this wrapper stores data in the app
 * representation (tainted) or the sandbox representation (tainted_volatile)
 * @tparam TFunc is the type of the "original" function pointer of being exposed
 * as a callback. By original, this means if the callback has type
 * `tainted_libtest<int> foo(rlbox_sandbox_libtest& sandbox,
 * tainted_libtest<int> a)`, then TFunc must be `int (*)(int)`
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TFunc, typename TSbx>
class rlbox_callback_impl {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 private:
  rlbox_sandbox<TSbx>* mSandbox{0};

  /**
   * @brief The type of the callback function from the host side. The first
   * argument is a reference to the sandbox, and subsequent arguments ar
   * tainted. The return type is either void or a tainted.
   */
  using external_callback_t =
      decltype(callback_detail::external_callback_type_helper<TSbx>(
          std::declval<TFunc>()));

  external_callback_t mExternalCallback;

  /**
   * @brief Construct a new rlbox_callback object.
   * @param aSandbox is the sandbox for which the callback is registered. This
   * must not be null.
   * @details  This constructor is private as only rlbox_sandbox should be able
   * to create this object.
   */
  rlbox_callback_impl(rlbox_sandbox<TSbx>* aSandbox,
                      external_callback_t aExternalCallback)
      : mSandbox(aSandbox), mExternalCallback(aExternalCallback) {
    detail::dynamic_check(mSandbox != nullptr,
                          "Unexpected null sandbox when creating a callback");
  }
};

}  // namespace rlbox
