/**
 * @file rlbox_memory_mgmt.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the memory management wrappers such as
 * unique_ptr for tainted types.
 */

#pragma once

#include "rlbox_sandbox.hpp"

#include <cstdlib>
#include <stddef.h>
#include <type_traits>

namespace rlbox {

/**
 * @brief A memory management class the implements unique_ptr for tainted types.
 * @details Tainted types are not compatible with std::unique_ptr (as
 * std::unique_ptr requires the managed type to be a pointer while RLBox tainted
 * types manage pointers are structs). This class thus provides the an
 * unique_ptr compatible with tainted types. The class interface tries to follow
 * the std::unique_ptr as much as possible.
 *
 * Example usage below.
 * @code {.cpp}
 * rlbox_unique_ptr_test<int> u1 = make_unique_tainted<int>(sandbox);
 * @endcode
 *
 * @tparam T is the type of the pointer being managed. For example, to manage a
 * `tainted<int*>`, developers would use a `rlbox_unique_ptr<int>`
 * @tparam TSbx is the type of the sandbox plugin that represents the
 * underlying sandbox implementation.
 */
template <typename T, typename TSbx>
class rlbox_unique_ptr {
  /**
   * @brief The tainted type used by the underlying TSbx specification.
   * @tparam TPtr is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename TPtr>
  using tainted = typename TSbx::template tainted<TPtr>;

  /**
   * @brief The tainted_volatile type used by the underlying TSbx specification.
   * @tparam TPtr is the type of the data that is wrapped.
   * @tparam TSbx is the type of the sandbox plugin that represents the
   * underlying sandbox implementation.
   */
  template <typename TPtr>
  using tainted_volatile = typename TSbx::template tainted_volatile<TPtr>;

  /**
   * @brief Tainted pointer being managed by rlbox_unique_ptr
   */
  tainted<T*> ptr{nullptr};

  /**
   * @brief Sandbox that @ref ptr belongs to.
   */
  rlbox_sandbox<TSbx>* sandbox{nullptr};

  /**
   * @brief Internal functions that frees the pointer if not null.
   */
  inline void free_ptr() noexcept(noexcept(sandbox->free_in_sandbox(ptr))) {
    if (ptr) {
      sandbox->free_in_sandbox(ptr);
      ptr = nullptr;
      sandbox = nullptr;
    }
  }

  /**
   * @brief Internal function that calls free and suppresses any exceptions.
   */
  inline void free_ptr_noexcept() {
    using is_free_ptr_no_except = std::is_nothrow_invocable<
        decltype(&rlbox_unique_ptr<T, TSbx>::free_ptr),
        rlbox_unique_ptr<T, TSbx>*>;

    if constexpr (is_free_ptr_no_except::value) {
      free_ptr();
    } else {
      try {
        free_ptr();
      } catch (...) {
        std::abort();
      }
    }
  }

 public:
  /**
   * @brief Construct a new rlbox unique ptr object set to null
   */
  inline rlbox_unique_ptr() noexcept = default;
  /**
   * @brief Construct a new rlbox unique ptr object set to null
   */
  inline rlbox_unique_ptr(const std::nullptr_t& /* unused */) noexcept {}
  /**
   * @brief Construct a new rlbox unique ptr object from a tainted pointer
   * @param aPtr is the pointer being managed
   * @param aSandbox is the sandbox to which the pointer belongs
   */
  inline rlbox_unique_ptr(tainted<T*> aPtr,
                          rlbox_sandbox<TSbx>& aSandbox) noexcept
      : ptr(aPtr), sandbox(&aSandbox) {}
  /**
   * @brief Construct a new rlbox unique ptr object from a tainted_volatile
   * pointer
   * @param aPtr is the pointer being managed
   * @param aSandbox is the sandbox to which the pointer belongs
   */
  inline rlbox_unique_ptr(tainted_volatile<T*> aPtr,
                          rlbox_sandbox<TSbx>& aSandbox) noexcept
      : ptr(aPtr), sandbox(&aSandbox) {}

  inline rlbox_unique_ptr(const rlbox_unique_ptr<T, TSbx>& aOther) = delete;
  /**
   * @brief Move constructor: Construct a new rlbox_unique_ptr object
   * @param aOther is the rhs argument
   */
  inline rlbox_unique_ptr(rlbox_unique_ptr<T, TSbx>&& aOther) noexcept
      : ptr(aOther.ptr), sandbox(aOther.sandbox) {
    aOther.release();
  }
  /**
   * @brief Destroy the rlbox unique ptr object and free the underlying pointer
   */
  inline ~rlbox_unique_ptr() noexcept { free_ptr_noexcept(); }

  inline rlbox_unique_ptr& operator=(const rlbox_unique_ptr<T, TSbx>& aOther) =
      delete;
  /**
   * @brief Set the value of the pointer being ,anaged to null
   */
  inline rlbox_unique_ptr& operator=(
      const std::nullptr_t& /* unused */) noexcept(noexcept(free_ptr())) {
    free_ptr();
    return *this;
  }
  /**
   * @brief Implement the move assignment operator for rlbox_unique_ptr
   * @param aOther is the rhs argument
   */
  inline rlbox_unique_ptr& operator=(
      rlbox_unique_ptr<T, TSbx>&& aOther) noexcept(noexcept(free_ptr())) {
    free_ptr();
    std::swap(ptr, aOther.ptr);
    std::swap(sandbox, aOther.sandbox);
    return *this;
  }

  /**
   * @brief Get the value of the pointer being managed
   * @return tainted<T*> is the pointer being managed by this rlbox_unique_ptr
   */
  [[nodiscard]] inline tainted<T*> get() const noexcept { return ptr; }
  /**
   * @brief Get the sandbox of the pointer being managed
   * @return rlbox_sandbox<TSbx>* is a pointer being managed by this
   * rlbox_unique_ptr. This could be null if the @ref ptr being managed is null.
   */
  [[nodiscard]] inline rlbox_sandbox<TSbx>* get_sandbox() const noexcept {
    return sandbox;
  }

  /**
   * @brief Free any existing pointer being managed and replace that with a
   * new pointer that must be managed
   *
   * @param aPtr is the new pointer to be managed
   * @param aSandbox is the sandbox to which `aPtr` belongs to
   */
  inline void reset(tainted<T*> aPtr, rlbox_sandbox<TSbx>& aSandbox) noexcept(
      noexcept(free_ptr())) {
    free_ptr();
    ptr = aPtr;
    sandbox = &aSandbox;
  }
  /**
   * @brief Free any existing pointer being managed and set the managed pointer
   * to null
   */
  inline void reset(const std::nullptr_t& /* unused */) noexcept(
      noexcept(free_ptr())) {
    free_ptr();
  }
  /**
   * @brief Release the pointer being managed to the caller. This pointer will
   * no longer by cleaned up rlbox_unique_ptr.
   * @return tainted<T*> was the the pointer that was previously being managed
   * by this instance
   */
  inline tainted<T*> release() noexcept {
    tainted<T*> copy = ptr;
    ptr = nullptr;
    sandbox = nullptr;
    return copy;
  }
  /**
   * @brief Swap the pointer being managed with the pointer from a different
   * rlbox_unique_ptr
   * @param aOther is the rhs argument
   */
  inline void swap(rlbox_unique_ptr<T, TSbx>& aOther) noexcept {
    std::swap(ptr, aOther.ptr);
    std::swap(sandbox, aOther.sandbox);
  }

  inline auto& operator*() const
      noexcept(noexcept(ptr.tainted<T*>::operator*())) {
    return ptr.tainted<T*>::operator*();
  }

  // inline auto operator->() const
  //     noexcept(noexcept(ptr.tainted<T*>::operator->())) {
  //   return ptr.tainted<T*>::operator->();
  // }
};

/**
 * @brief Create a new pointer of type `tainted<t*> and manage this pointer with
 * @ref rlbox::rlbox_unique_ptr
 *
 * @tparam T is the type of the pointer to be created
 * @tparam TSbx is the type of the sandbox to which the pointer belongs
 * @param aSandbox is the sandbox to which the pointer belongs
 * @return rlbox_unique_ptr<T, TSbx> is the managed unique pointer of type
 * `tainted<T*>`
 */
template <typename T, typename TSbx>
rlbox_unique_ptr<T, TSbx> make_unique_tainted(rlbox_sandbox<TSbx>& aSandbox) {
  auto ptr = aSandbox.template malloc_in_sandbox<T>();
  rlbox_unique_ptr<T, TSbx> ret(std::move(ptr), aSandbox);
  return ret;
}

}  // namespace rlbox
