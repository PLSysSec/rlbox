#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

#include <type_traits>
#include <utility>

#include "rlbox_struct_support.hpp"
#include "rlbox_types.hpp"

namespace rlbox {

namespace callback_detail {

  // Compute the expected type of the callback
  template<typename T_Sbx, typename T_Ret, typename... T_Args>
  using T_Cb =
    std::conditional_t<std::is_void_v<T_Ret>, void, tainted<T_Ret, T_Sbx>> (*)(
      RLBoxSandbox<T_Sbx>&,
      tainted<T_Args, T_Sbx>...);

  template<typename T_Sbx, typename T_Ret, typename... T_Args>
  T_Cb<T_Sbx, T_Ret, T_Args...> callback_type_helper(T_Ret (*)(T_Args...));

  // Compute the expected type of the interceptor
  template<typename T_Sbx, typename T_Ret, typename... T_Args>
  using T_I = detail::convert_to_sandbox_equivalent_t<T_Ret, T_Sbx> (*)(
    detail::convert_to_sandbox_equivalent_t<T_Args, T_Sbx>...);

  template<typename T_Sbx, typename T_Ret, typename... T_Args>
  T_I<T_Sbx, T_Ret, T_Args...> interceptor_type_helper(T_Ret (*)(T_Args...));
}

template<typename T, typename T_Sbx>
class sandbox_callback
  : public sandbox_wrapper_base
  , public sandbox_wrapper_base_of<T*>
{
private:
  T_Sbx* sandbox;

  using T_Callback =
    decltype(callback_detail::callback_type_helper<T_Sbx>(std::declval<T>()));
  T_Callback callback;

  // The interceptor is the function that runs between the sandbox invoking the
  // callback and the actual callback running The interceptor is responsible for
  // wrapping and converting callback arguments, returns etc. to their
  // appropriate representations
  using T_Interceptor = decltype(
    callback_detail::interceptor_type_helper<T_Sbx>(std::declval<T>()));
  T_Interceptor callback_interceptor;

  // The trampoline is the internal sandbox representation of the callback
  // Depending on the sandbox type, this could be the callback pointer directly
  // or a trampoline function that gates exits from the sandbox.
  using T_Trampoline = detail::convert_to_sandbox_equivalent_t<T, T_Sbx>;
  T_Trampoline callback_trampoline;

  inline void move_obj(sandbox_callback&& other)
  {
    sandbox = other.sandbox;
    callback = other.callback;
    callback_interceptor = other.callback_interceptor;
    callback_trampoline = other.callback_trampoline;
    other.sandbox = nullptr;
    other.callback = nullptr;
    other.callback_interceptor = nullptr;
    other.callback_trampoline = 0;
  }

public:
  sandbox_callback(T_Sbx* p_sandbox,
                   T_Callback p_callback,
                   T_Interceptor p_callback_interceptor,
                   T_Trampoline p_callback_trampoline)
    : sandbox(p_sandbox)
    , callback(p_callback)
    , callback_interceptor(p_callback_interceptor)
    , callback_trampoline(p_callback_trampoline)
  {
    detail::dynamic_check(sandbox != nullptr,
                          "Unexpected null sandbox when creating a callback");
  }

  sandbox_callback(sandbox_callback&& other)
  {
    move_obj(std::forward<sandbox_callback>(other));
  }

  inline sandbox_callback& operator=(sandbox_callback&& other)
  {
    if (this != &other) {
      move_obj(std::forward<sandbox_callback>(other));
    }
    return *this;
  }

  //   void unregister()
  //   {
  //     if (callback != nullptr) {
  //       sandbox->template
  //       impl_UnregisterCallback<T>(stateObject->actualCallback); delete
  //       stateObject; this->sandbox = nullptr; this->callback =
  //       nullptr; this->stateObject = nullptr;
  //     }
  //   }

  //   ~sandbox_callback() { unregister(); }

  inline T_Callback UNSAFE_Unverified() const noexcept { return callback; }
  inline T_Trampoline UNSAFE_Sandboxed() const noexcept
  {
    return callback_trampoline;
  }
};

}