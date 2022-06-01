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
#include <stddef.h>
#include <stdint.h>
#include <utility>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_checked_arithmetic.hpp"
#include "rlbox_error_handling.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_types.hpp"
// IWYU doesn't seem to recognize the call to impl_promote_integer_types_t so
// force IWYU to keep the next include
#include "rlbox_wrapper_traits.hpp"  // IWYU pragma: keep

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
   * after destruction. We only perform checks suring create_sandbox,
   * detroy_sandbox and register_callback where they will not add too much
   * overhead. Even this limited checking can be diabled through the macro
   * RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
   */
  enum class create_status { NOT_CREATED, INITIALIZING, DESTRUCTING, CREATED };

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
  /**
   * @brief This variable tracks the creation status of the sandbox instance
   */
  std::atomic<create_status> sandbox_created = create_status::NOT_CREATED;
#endif

 public:
  /**
   * @brief The tainted type used by the underlying TSbx specification.
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted = typename TSbx::template tainted<T>;

  /**
   * @brief The tainted_volatile type used by the underlying TSbx specification.
   * @tparam T is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename T>
  using tainted_volatile = typename TSbx::template tainted_volatile<T>;

  template <typename T>
  using base_types_convertor_tsbx = detail::rlbox_base_types_convertor<T, TSbx>;

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
    bool success = sandbox_created.compare_exchange_strong(
        expected, create_status::INITIALIZING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already created/is being "
        "created concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    rlbox_status_code ret =
        this->impl_create_sandbox(std::forward<TArgs>(aArgs)...);

#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    if (ret == rlbox_status_code::SUCCESS) {
      sandbox_created.store(create_status::CREATED);
    } else {
      sandbox_created.store(create_status::NOT_CREATED);
    }
#endif

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
    bool success = sandbox_created.compare_exchange_strong(
        expected, create_status::DESTRUCTING /* desired */);
    detail::dynamic_check(
        success,
        "create_sandbox called when sandbox already destroyed/is being "
        "destroyed concurrently");
#endif
    // Simply pass on the call to the underlying plugin as this operation is
    // specific to the plugin.
    rlbox_status_code ret = this->impl_destroy_sandbox();
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    sandbox_created.store(create_status::NOT_CREATED);
#endif
    return ret;
  }

  /**
   * @brief Convert the pointer that is in sandbox representation (ABI) to the
   * host representation
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
   * sandbox representation
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
      // NOLINTNEXTLINE(hicpp-use-nullptr, modernize-use-nullptr)
      base_types_convertor_tsbx<T> ret{0};
      return ret;
    }

    auto ret = this->template impl_get_sandboxed_pointer<T>(aPtr);
    return ret;
  }

  // template<typename T>
  // static inline T get_unsandboxed_pointer_no_ctx(
  //   convert_to_sandbox_equivalent_nonclass_t<T> p,
  //   const void* example_unsandboxed_ptr)
  // {
  //   static_assert(std::is_pointer_v<T>);
  //   if (p == 0) {
  //     return nullptr;
  //   }
  //   auto ret = T_Sbx::template impl_get_unsandboxed_pointer_no_ctx<T>(
  //     p, example_unsandboxed_ptr, find_sandbox_from_example);
  //   return reinterpret_cast<T>(ret);
  // }

  // template<typename T>
  // static inline convert_to_sandbox_equivalent_nonclass_t<T>
  // get_sandboxed_pointer_no_ctx(const void* p,
  //                              const void* example_unsandboxed_ptr)
  // {
  //   static_assert(std::is_pointer_v<T>);
  //   if (p == nullptr) {
  //     return 0;
  //   }
  //   return T_Sbx::template impl_get_sandboxed_pointer_no_ctx<T>(
  //     p, example_unsandboxed_ptr, find_sandbox_from_example);
  // }

  //  private:
  //   template <typename TArg>
  //   auto invoke_process_param(TArg&& aArg) {
  //     return std::forward<TArg>(aArg);
  //   }

  //   template <typename TFunc, typename... TArgs>
  //   auto inline invoke_sandbox_function_helper(const char* aFuncName,
  //                                              void* aFuncPtr, TArgs&&...
  //                                              aArgs) {
  //     return this->template impl_invoke_with_func_ptr<TFunc>(
  //         reinterpret_cast<TFunc*>(aFuncPtr),
  //         invoke_process_param(aArgs)...);
  //   }

  //  public:
  //   template <typename TFunc, typename... TArgs>
  //   auto inline invoke_sandbox_function(const char* aFuncName, void*
  //   aFuncPtr,
  //                                       TArgs&&... aArgs) {
  //     constexpr bool abi_unchanged =
  //     detail::rlbox_base_types_unchanged_v<TSbx>;

  //     if constexpr (abi_unchanged) {
  //       using TPromoted =
  //           typename TSbx::template impl_promote_integer_types_t<TFunc>;
  //       return this->invoke_sandbox_function_helper<TPromoted>(
  //           aFuncName, aFuncPtr, std::forward<TArgs...>(aArgs...));
  //     } else {
  //       return this->invoke_sandbox_function_helper<TFunc>(
  //           aFuncName, aFuncPtr, std::forward<TArgs>(aArgs)...);
  //     }
  //   }

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
  size_t get_object_size_for_malloc() {
    // RLBox has to compute the size of the allocation in the sandbox's ABI.
    // When allocating structs/classes, and the sandbox ABI is not larger than
    // the host ABI (see @ref rlbox::detail::rlbox_base_types_not_larger_v),
    // then RLBox just uses the host ABI size for the allocation of the class as
    // it will always be greater than equal to the size required by the actual
    // allocation. This ensures that users of the API don't have to call
    // `rlbox_lib_load_classes` for each class allocated.
#ifndef RLBOX_DONT_OVERESTIMATE_CLASS_SIZES
    using TSbxRep = base_types_convertor_tsbx<T>;
    return sizeof(TSbxRep);
#else
    static_assert(detail::false_v<T>, RLBOX_NOT_IMPLEMENTED_MESSAGE);
    // Use a dummy non zero return
    return 1;
#endif
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
  inline tainted<T*> malloc_in_sandbox() {
    tainted<size_t> default_count(1);
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
  inline tainted<T*> malloc_in_sandbox(tainted<size_t> aCount) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    // Silently swallowing the failure is better here as RAII types may try to
    // malloc after sandbox destruction
    if (sandbox_created.load() != create_status::CREATED) {
      return tainted<T*>(nullptr);
    }
#endif
    size_t count_unwrapped = aCount.raw_host_rep();

    detail::dynamic_check(count_unwrapped != 0,
                          "Allocation of 0 bytes requested");

    size_t object_size = get_object_size_for_malloc<T>();
    auto total_size = rlbox::detail::checked_multiply<size_t>(
        object_size, count_unwrapped,
        "Allocation size computaion has overflowed");

    if constexpr (detail::has_member_impl_malloc_in_sandbox_v<TSbx>) {
      base_types_convertor_tsbx<T*> ptr_in_sandbox =
          this->template impl_malloc_in_sandbox<T>(total_size);
      T* ptr = get_unsandboxed_pointer<T*>(ptr_in_sandbox);

      auto ptr_start = reinterpret_cast<uintptr_t>(ptr);
      auto ptr_end = detail::checked_add<uintptr_t>(
          ptr_start, (total_size - 1),
          "Pointer end computation has overflowed");

      bool start_in_bounds = this->impl_is_pointer_in_sandbox_memory(
          reinterpret_cast<void*>(ptr_start));
      bool end_in_bounds = this->impl_is_pointer_in_sandbox_memory(
          reinterpret_cast<void*>(ptr_end));

      detail::dynamic_check(
          start_in_bounds && end_in_bounds,
          "Malloc returned pointer outside the sandbox memory");

      tainted<T*> ret(ptr);
      return ret;
    } else {
      // Use sandbox_invoke call malloc in the sandbox code
      static_assert(detail::false_v<T>, RLBOX_NOT_IMPLEMENTED_MESSAGE);
      // Use dummy return
      tainted<T*> ret(nullptr);
      return ret;
    }
  }

  /**
   * @brief Free the memory referenced by the tainted pointer.
   * @tparam T is the type of the pointer you want to free.
   * @param aPtr is the pointer to sandbox memory to free.
   */
  template <typename T>
  inline void free_in_sandbox(tainted<T*> aPtr) {
#ifndef RLBOX_DISABLE_SANDBOX_CREATED_CHECKS
    // Silently swallowing the failure is better here as RAII types may try to
    // free after sandbox destruction
    if (sandbox_created.load() != create_status::CREATED) {
      return;
    }
#endif

    this->template impl_free_in_sandbox<T>(aPtr.raw_sandbox_rep(*this));
  }
};

#ifdef RLBOX_USE_STATIC_CALLS

#  define rlbox_lookup_symbol_helper(rlbox_plugin_lookup_macro, func_name) \
    rlbox_plugin_lookup_macro(func_name)

/**
 * @brief rlbox_lookup_symbol is helper to looks up function addresses
 * - via simple function names if `RLBOX_USE_STATIC_CALLS` is defined
 * - using symbol lookup if `RLBOX_USE_STATIC_CALLS` is not defined
 */
#  define rlbox_lookup_symbol(sandbox, func_name) \
    rlbox_lookup_symbol_helper(RLBOX_USE_STATIC_CALLS(), func_name)

#else
/**
 * @brief rlbox_lookup_symbol is helper to looks up function addresses
 * - via simple function names if `RLBOX_USE_STATIC_CALLS` is defined
 * - using symbol lookup if `RLBOX_USE_STATIC_CALLS` is not defined
 */
#  define rlbox_lookup_symbol(sandbox, func_name) \
    sandbox.dynamic_symbol_lookup(#func_name)

#endif

/**
 * @brief API used to invoke sandbox functions while also specifying the
 * function type. The parameters are expected to be rlbox wrapper types like
 * `tainted' types or `rlbox::rlbox_callback` types.
 *
 * This macro is used internally in most cases.
 *
 * However, end users must invoke sandbox functions with this macro IFF the
 * sandboxing technology both
 *   - Changes the ABI of types like int, long etc.
 *   - Does not expose the target type of compiled code
 *
 * This means
 *   - Users of the rlbox_noop_sandbox (does not change ABI) will NOT need this
 *   - Users of the rlbox_wasm2c_sandbox (changes the ABI, but exposes the
 * target type of the function) will NOT need this
 *   - Users of the rlbox_nacl_sandbox (changes the ABI, does not expose the
 * target type of the function) will need this
 *
 * For e.g.,
 * @code {.cpp}
 * auto result = sandbox_invoke_with_func_type(sandbox, int(int, int), lib_add,
 * 3, 4);
 * @endcode
 *
 * This can also be used with rlbox integer types to currently account for the
 * ABI changes
 * @code {.cpp}
 * auto result = sandbox_invoke_with_func_type(sandbox,
 * rlbox_uint32_t(rlbox_uint32_t, rlbox_uint32_t), lib_add, 3, 4);
 * @endcode
 * @details This macro ultimately forwards the call to a method on @ref
 * rlbox::rlbox_sandbox along with  a stringified version of the
 * function name
 */
#define sandbox_invoke_with_func_type(sandbox, func_type, func_name, ...) \
  sandbox.invoke_sandbox_function<func_type>(                             \
      #func_name, rlbox_lookup_symbol(func_name), ##_VA_ARGS_...)

/**
 * @brief API used to invoke sandbox functions. The parameters are expected to
 * be rlbox wrapper types like `tainted' types or `rlbox::rlbox_callback` types.
 *
 * For e.g.,
 * @code {.cpp}
 * auto result = sandbox_invoke(sandbox, lib_add, 3, 4);
 * @endcode
 * @details This macro ultimately forwards the call to a method on @ref
 * rlbox::rlbox_sandbox along with the function type.
 */
#define sandbox_invoke(sandbox, func_name, ...)                          \
  sandbox_invoke_with_func_type(sandbox, decltype(func_name), func_name, \
                                ##_VA_ARGS_...)
}  // namespace rlbox