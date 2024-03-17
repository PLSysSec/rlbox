/**
 * @file rlbox_sandbox.hpp
 * @copyright This project is released under the MIT License.
 * You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the rlbox_sandbox class.
 */

#pragma once

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
#  include <atomic>
#endif
#include <cstddef>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_checked_arithmetic.hpp"
#include "rlbox_error_handling.hpp"
#include "rlbox_function_traits.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {

/**
 * @brief An rlbox sandbox type that provides APIs to invoke functions in the
 * sandboxed component, expose callbacks, retrieve information about the sandbox
 * memory etc.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation. For the "null" sandbox, the value is
 * rlbox_noop_sandbox.
 */
template <typename TSbx>
class rlbox_sandbox : protected TSbx {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 private:
  /**
   * @brief This type tracks the state of sandbox creation and is for internal
   * use only. This state is checked prior to any operations on the sandbox from
   * the host program.
   * @details We should ideally check this state to see if the sandbox is in the
   * created state during sandbox operations such as invoking functions.
   * However, it is expensive to check in APIs such as sandbox_invoke or in the
   * callback_interceptor. In general, we leave it up to the user to ensure APIs
   * such as sandbox_invoke are never called prior to sandbox construction or
   * after destruction. We only perform checks during create_sandbox,
   * destroy_sandbox and register_callback where they will not add too much
   * overhead. Even this limited checking can be diabled through the macro
   * RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
   */
  enum class create_status { NOT_CREATED, INITIALIZING, DESTRUCTING, CREATED };

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
  /**
   * @brief This variable tracks the creation status of the sandbox instance
   */
  std::atomic<create_status> mSandboxCreated = create_status::NOT_CREATED;
#endif

  /**
   * @brief A list of live sandboxes of this type. This list is only maintained
   * if necessary by internals of this class. See detailed comments below for
   * more information.
   *
   * @details This list is maintained if @ref
   * rlbox::rlbox_sandbox::get_unsandboxed_pointer_with_example or @ref
   * rlbox::rlbox_sandbox::get_sandboxed_pointer_with_example requires the list
   * of all active sandboxes for their operation. Those two function require
   * this list if the underlying sandbox plugin `TSbx` does not implement
   * `impl_get_unsandboxed_pointer_with_example' or
   * `impl_get_sandboxed_pointer_with_example'
   */
  static inline std::set<rlbox_sandbox*> mSandboxes;

  /**
   * @brief A read-write mutex to guard access to @ref
   * rlbox::rlbox_sandbox::mSandboxes
   */
  static inline std::shared_mutex mSandboxesMutex;

 public:
  template <typename T>
  using base_types_convertor_tsbx = detail::rlbox_base_types_convertor<T, TSbx>;

  /**
   * @brief Get the inner sandbox implementation object
   * @return TSbx* which is the instance of the inner object
   */
  TSbx* get_inner_sandbox_impl() { return this; }

  /**
   * @brief API invoked to initialize a new sandbox. This function should be run
   * to completion before any functions are invoked in this sandbox.
   *
   * @tparam TArgs is the type of the sandbox creation parameters. The value
   * here is specific to the plugin implementation.
   * @param aArgs specifies any sandbox creation parameters. The value here is
   * specific to the plugin implementation.
   * @return rlbox_status_code indicates whether this function succeeded
   */
  template <typename... TArgs>
  rlbox_status_code create_sandbox(TArgs... aArgs) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    auto expected = create_status::NOT_CREATED;
    const bool success = mSandboxCreated.compare_exchange_strong(
        expected, create_status::INITIALIZING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already created/is being "
        "created concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    const rlbox_status_code ret =
        this->impl_create_sandbox(std::forward<TArgs>(aArgs)...);

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    if (ret == rlbox_status_code::SUCCESS) {
      mSandboxCreated.store(create_status::CREATED);
    } else {
      mSandboxCreated.store(create_status::NOT_CREATED);
    }
#endif

    if constexpr (
        !detail::has_member_impl_get_unsandboxed_pointer_with_example_v<TSbx> ||
        !detail::has_member_impl_get_sandboxed_pointer_with_example_v<TSbx> ||
        !detail::has_member_impl_is_pointer_in_sandbox_memory_with_example_v<
            TSbx>) {
      if (ret == rlbox_status_code::SUCCESS) {
        const std::unique_lock lock(mSandboxesMutex);
        mSandboxes.insert(this);
      }
    }

    return ret;
  }

  /**
   * @brief Destroy the sandbox. This function should be called only after a
   * successful call to create_sandbox. After destruction, the application
   * should assume the sandbox no longer has memory and can no longer execute
   * function calls.
   *
   * @return rlbox_status_code indicates whether this function succeeded
   */
  rlbox_status_code destroy_sandbox() {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    auto expected = create_status::CREATED;
    const bool success = mSandboxCreated.compare_exchange_strong(
        expected, create_status::DESTRUCTING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already destroyed/is being "
        "destroyed concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    const rlbox_status_code ret = this->impl_destroy_sandbox();
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    mSandboxCreated.store(create_status::NOT_CREATED);
#endif

    if constexpr (
        !detail::has_member_impl_get_unsandboxed_pointer_with_example_v<TSbx> ||
        !detail::has_member_impl_get_sandboxed_pointer_with_example_v<TSbx> ||
        !detail::has_member_impl_is_pointer_in_sandbox_memory_with_example_v<
            TSbx>) {
      const std::unique_lock lock(mSandboxesMutex);
      mSandboxes.erase(this);
    }
    return ret;
  }

  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] inline bool is_pointer_in_sandbox_memory(
      T aPtr) const noexcept {
    if (!aPtr) {
      return true;
    }
    const bool ret = this->template impl_is_pointer_in_sandbox_memory<T>(aPtr);
    return ret;
  }

  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] static inline bool is_pointer_in_sandbox_memory_with_example(
      T aPtr, const void* aEgUnsbxedPtr) {
    if (!aPtr) {
      return true;
    }
    detail::dynamic_check(aEgUnsbxedPtr != nullptr,
                          "Internal error: received a null example "
                          "pointer. " RLBOX_FILE_BUG_MESSAGE);

    if constexpr (
        detail::has_member_impl_is_pointer_in_sandbox_memory_with_example_v<
            TSbx>) {
      auto ret =
          TSbx::template impl_is_pointer_in_sandbox_memory_with_example<T>(
              aPtr, aEgUnsbxedPtr);
      return ret;
    } else {
      const std::shared_lock lock(mSandboxesMutex);

      for (rlbox_sandbox* sandbox : mSandboxes) {
        if (sandbox->is_pointer_in_sandbox_memory(aEgUnsbxedPtr)) {
          return sandbox->template impl_is_pointer_in_sandbox_memory<T>(aPtr);
        }
      }

      // this function does not return
      detail::error_occured(
          "Internal error: could not find the sandbox belonging to a "
          "pointer. " RLBOX_FILE_BUG_MESSAGE);
    }
  }

  /**
   * @brief Convert the pointer that is in sandbox representation (ABI) to the
   * host representation. This function automatically will abort if the provided
   * pointer is out of range of the sandbox.
   * @tparam T is the type of the of the object. `T` must be a pointer.
   * @tparam RLBOX_REQUIRE checks to see if `T` is a pointer
   * @param aPtr is the pointer in sandbox representation
   * @return T is the return value which is the pointer in the host
   * representation
   */
  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] inline T get_unsandboxed_pointer(
      base_types_convertor_tsbx<T> aPtr) const {
    if (!aPtr) {
      return nullptr;
    }

    auto ret = this->template impl_get_unsandboxed_pointer<T>(aPtr);
    return ret;
  }

  /**
   * @brief Convert the pointer that is in host representation (ABI) to the
   * sandbox representation. This function automatically will abort if the
   * provided pointer is out of range of the sandbox.
   * @tparam T is the type of the of the object. `T` must be a pointer.
   * @tparam RLBOX_REQUIRE checks to see if `T` is a pointer
   * @param aPtr is the pointer in host representation
   * @return T is the return value which is the pointer in the sandbox
   * representation
   */
  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] inline base_types_convertor_tsbx<T> get_sandboxed_pointer(
      T aPtr) const {
    if (!aPtr) {
      const base_types_convertor_tsbx<T> ret{0};
      return ret;
    }

    auto ret = this->template impl_get_sandboxed_pointer<T>(aPtr);
    return ret;
  }

  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] static inline T get_unsandboxed_pointer_with_example(
      base_types_convertor_tsbx<T> aPtr, const void* aEgUnsbxedPtr) {
    if (!aPtr) {
      return nullptr;
    }

    detail::dynamic_check(aEgUnsbxedPtr != nullptr,
                          "Internal error: received a null example "
                          "pointer. " RLBOX_FILE_BUG_MESSAGE);

    if constexpr (detail::
                      has_member_impl_get_unsandboxed_pointer_with_example_v<
                          TSbx>) {
      auto ret = TSbx::template impl_get_unsandboxed_pointer_with_example<T>(
          aPtr, aEgUnsbxedPtr);
      return ret;
    } else {
      const std::shared_lock lock(mSandboxesMutex);

      for (rlbox_sandbox* sandbox : mSandboxes) {
        if (sandbox->is_pointer_in_sandbox_memory(aEgUnsbxedPtr)) {
          return sandbox->template get_unsandboxed_pointer<T>(aPtr);
        }
      }

      // this function does not return
      detail::error_occured(
          "Internal error: could not find the sandbox belonging to a "
          "pointer. " RLBOX_FILE_BUG_MESSAGE);
    }
  }

  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  [[nodiscard]] static inline base_types_convertor_tsbx<T>
  get_sandboxed_pointer_with_example(T aPtr, const void* aEgUnsbxedPtr) {
    if (!aPtr) {
      return 0;
    }

    detail::dynamic_check(aEgUnsbxedPtr != nullptr,
                          "Internal error: received a null example "
                          "pointer. " RLBOX_FILE_BUG_MESSAGE);

    if constexpr (detail::has_member_impl_get_sandboxed_pointer_with_example_v<
                      TSbx>) {
      auto ret = TSbx::template impl_get_sandboxed_pointer_with_example<T>(
          aPtr, aEgUnsbxedPtr);
      return ret;
    } else {
      const std::shared_lock lock(mSandboxesMutex);

      for (rlbox_sandbox* sandbox : mSandboxes) {
        if (sandbox->is_pointer_in_sandbox_memory(aEgUnsbxedPtr)) {
          return sandbox->template get_sandboxed_pointer<T>(aPtr);
        }
      }

      // this function does not return
      detail::error_occured(
          "Internal error: could not find the sandbox belonging to a "
          "pointer. " RLBOX_FILE_BUG_MESSAGE);
    }
  }

 private:
  template <typename TArg>
  inline auto invoke_process_param(TArg&& aArg) {
    using TNoRef = std::remove_reference_t<TArg>;

    if constexpr (detail::is_tainted_any_wrapper_v<TNoRef>) {
      if constexpr (!std::is_same_v<
                        TSbx, detail::rlbox_get_wrapper_sandbox_t<TNoRef>>) {
        rlbox_static_fail(
            TArg,
            "Mixing tainted data from a different sandbox types. This could "
            "happen when you are using 2 sandbox types, for example "
            "'rlbox_noop_sandbox' and 'rlbox_wasm2c_sandbox', and are passing "
            "tainted data from one sandbox as parameters into a function call "
            "to the other sandbox. This is not allowed, unwrap the tainted "
            "data with copy_and_verify or other unwrapping APIs first.\n");
      }
    } else if constexpr (!std::is_constructible_v<tainted<TNoRef, TSbx>,
                                                  TNoRef>) {
      rlbox_static_fail(
          TArg,
          "Arguments to a sandbox function call should either be values easily "
          "convertible to tainted like primitives integers and nullptr, or  "
          "wrapped types like tainted, callbacks etc.");
    }

    if constexpr (detail::is_tainted_any_wrapper_v<TNoRef>) {
      return aArg.UNSAFE_sandboxed(*this);
    } else {
      const tainted<TNoRef, TSbx> val(aArg);
      return val.UNSAFE_sandboxed(*this);
    }
  }

  template <typename T>
  using get_param_type_t = detail::rlbox_stdint_to_stdint_t<
      detail::rlbox_remove_wrapper_t<std::remove_reference_t<T>>>;

 public:
  template <typename TFunc, typename... TArgs>
  inline auto INTERNAL_invoke_sandbox_function(
      [[maybe_unused]] const char* aFuncName, void* aFuncPtr,
      TArgs&&... aArgs) {
    static_assert(std::is_invocable_v<TFunc, get_param_type_t<TArgs>...>,
                  "Mismatched arguments types for function");

    // Convert the function argument and return types to the sandbox equivalent
    // types
    using TFuncConv =
        detail::func_type_converter_t<TFunc, base_types_convertor_tsbx>;
    using TRet = detail::return_type_t<TFunc>;

    if constexpr (std::is_void_v<TRet>) {
      return this->template impl_invoke_with_func_ptr<TFuncConv>(
          aFuncPtr, invoke_process_param(aArgs)...);
    } else if constexpr (std::is_constructible_v<tainted<TRet, TSbx>, TRet>) {
      tainted<TRet, TSbx> ret =
          this->template impl_invoke_with_func_ptr<TFuncConv>(
              aFuncPtr, invoke_process_param(aArgs)...);
      return ret;
    } else if constexpr (std::is_pointer_v<TRet>) {
      const base_types_convertor_tsbx<TRet> ptr_sbx_rep =
          this->template impl_invoke_with_func_ptr<TFuncConv>(
              aFuncPtr, invoke_process_param(aArgs)...);
      TRet ptr = get_unsandboxed_pointer<TRet>(ptr_sbx_rep);

      const size_t object_size = get_object_size_upperbound<TRet>();
      tainted<TRet, TSbx> ret = get_tainted_from_raw_ptr(ptr, object_size);
      return ret;
    } else {
      static_assert(detail::false_v<TFunc>, "Not implemented");
    }
  }

 private:
  /**
   * @brief Construct a tainted pointer, after checking that the given pointer
   * refers to an object inside the sandbox memory.
   *
   * @tparam T is the type of the pointer you want to create.
   * @tparam RLBOX_REQUIRE checks to see if `T` is a pointer
   * @param aPtr is the raw pointer that should refer to an object in the
   * sandbox memory
   * @param aSize is the size of the object/objects that is being referred to
   * @return tainted<T, TSbx> is the given pointer as a tainted value
   */
  template <typename T, RLBOX_REQUIRE(std::is_pointer_v<T>)>
  tainted<T, TSbx> get_tainted_from_raw_ptr(T aPtr, size_t aSize) {
    auto ptr_start = reinterpret_cast<uintptr_t>(aPtr);
    auto ptr_end = detail::checked_add<uintptr_t>(
        ptr_start, (aSize - 1), "Pointer end computation has overflowed");

    /// \todo Fix. This assumes the memory is contiguous. Else, start may be
    /// inside, end may be inside, but the middle may not be inside.

    const bool start_in_bounds = this->impl_is_pointer_in_sandbox_memory(
        reinterpret_cast<void*>(ptr_start));
    const bool end_in_bounds = this->impl_is_pointer_in_sandbox_memory(
        reinterpret_cast<void*>(ptr_end));

    detail::dynamic_check(start_in_bounds && end_in_bounds,
                          "Trying to convert a raw pointer which is outside "
                          "the sandbox to a tainted pointer");

    auto ret = tainted<T, TSbx>::from_unchecked_raw_pointer(aPtr);
    return ret;
  }

 public:
  /**
   * @brief Get the size of the object of type `T` in the sandbox representation
   * @details This function uses the following rules
   * - If `T` is an fundamental, enum, or pointer type, we use the ABI as
   * specified in sandbox plugin `TSbx`.
   * - If `T` is a class type
   *    - If the ABI of the sandbox plugin `TSbx` is not larger than the
   * host ABI, then we just use the host ABI size
   *    - Else, we query the sandbox for the size of the given class. This
   * requires the user to include the class `T` as part of the call to
   * rlbox_define_library_structs
   * @tparam T is the type for which we are getting the sandbox representation
   * size
   * @return size_t is the size of type T in the sandbox representation
   */
  template <typename T>
  size_t get_object_size_upperbound() {
    if constexpr (!std::is_class_v<T> || detail::is_rlbox_stdint_type_v<T>) {
      using TSbxRep = base_types_convertor_tsbx<T>;
      return sizeof(TSbxRep);
    } else {
      // RLBox has to compute the size of the allocation in the sandbox's ABI.
      // When allocating structs/classes, and the sandbox ABI is not larger than
      // the host ABI (see @ref rlbox::detail::rlbox_base_types_not_larger_v),
      // then RLBox just uses the host ABI size for the allocation of the class
      // as it will always be greater than equal to the size required by the
      // actual allocation. This ensures that users of the API don't have to
      // call `rlbox_lib_load_classes` for each class allocated.
#ifndef RLBOX_DONT_OVERESTIMATE_CLASS_SIZES
      constexpr bool can_overestimate_check1 = true;
#else
      constexpr bool can_overestimate_check1 = false;
#endif

      constexpr bool can_overestimate_check2 =
          detail::rlbox_base_types_not_larger_v<TSbx>;

      if constexpr (can_overestimate_check1 && can_overestimate_check2) {
        using TSbxRep = base_types_convertor_tsbx<T>;
        return sizeof(TSbxRep);
      } else {
        // Use rlbox_static_assert here instead of static_assert as there is a
        // testcase for this code
        rlbox_static_fail(T, RLBOX_NOT_IMPLEMENTED_MESSAGE);
        // Use a dummy non zero return
        return 1;
      }
    }
  }

  /**
   * @brief Allocate a new pointer that is accessible to both the application
   * and sandbox. The pointer is allocated in sandbox memory.
   * @tparam T is the type of the pointer you want to create. If T=int, this
   * would return a pointer to an int.
   * @return tainted<T*> is tainted pointer accessible to the application and
   * sandbox.
   */
  template <typename T>
  inline tainted<T*, TSbx> malloc_in_sandbox() {
    const tainted<size_t, TSbx> default_count(1);
    return malloc_in_sandbox<T>(default_count);
  }

  /**
   * @brief Allocate an array that is accessible to both the application
   * and sandbox. The pointer is allocated in sandbox memory.
   * @tparam T is the type of the array elements you want to create. If T=int,
   * this would return a pointer to an array of ints.
   * @param aCount is the number of array elements to allocate.
   * @return tainted<T*> Tainted pointer accessible to the application
   * and sandbox.
   */
  template <typename T>
  inline tainted<T*, TSbx> malloc_in_sandbox(tainted<size_t, TSbx> aCount) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    detail::dynamic_check(mSandboxCreated.load() == create_status::CREATED,
                          "Sandbox not created");
#endif
    const size_t count_unwrapped = aCount.raw_host_rep();

    detail::dynamic_check(count_unwrapped != 0,
                          "Allocation of 0 bytes requested");

    const size_t object_size = get_object_size_upperbound<T>();
    auto total_size = rlbox::detail::checked_multiply<size_t>(
        object_size, count_unwrapped,
        "Allocation size computation has overflowed");

    if constexpr (detail::has_member_impl_malloc_in_sandbox_v<TSbx>) {
      const base_types_convertor_tsbx<T*> ptr_sbx_rep =
          this->template impl_malloc_in_sandbox<T>(total_size);
      T* ptr = get_unsandboxed_pointer<T*>(ptr_sbx_rep);

      tainted<T*, TSbx> ret = get_tainted_from_raw_ptr(ptr, total_size);
      return ret;
    } else {
      /// \todo Use sandbox_invoke call malloc in the sandbox code
      static_assert(detail::false_v<T>, RLBOX_NOT_IMPLEMENTED_MESSAGE);
      // Use dummy return
      tainted<T*, TSbx> ret(nullptr);
      return ret;
    }
  }

  /**
   * @brief Free the memory referenced by the tainted pointer.
   * @tparam TWrap is the other wrapper type
   * @tparam T is the type of the pointer you want to free.
   * @param aPtr is the pointer to sandbox memory to free.
   */
  template <template <bool, typename, typename, typename...> typename TWrap,
            bool TUseAppRep, typename T, typename... TExtra,
            RLBOX_REQUIRE(detail::is_tainted_any_wrapper_v<
                          TWrap<TUseAppRep, T, TSbx, TExtra...>>&&
                              std::is_pointer_v<T>)>
  inline void free_in_sandbox(TWrap<TUseAppRep, T, TSbx, TExtra...> aPtr) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    detail::dynamic_check(mSandboxCreated.load() == create_status::CREATED,
                          "Sandbox not created");
#endif

    this->template impl_free_in_sandbox<T>(aPtr.raw_sandbox_rep(*this));
  }
};

}  // namespace rlbox