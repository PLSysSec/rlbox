/**
 * @file rlbox_tainted_volatile.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_volatile pointer type.
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "rlbox_abi_conversion.hpp"
#include "rlbox_helpers.hpp"
#include "rlbox_sandbox_plugin_base.hpp"
#include "rlbox_types.hpp"
#include "rlbox_wrapper_traits.hpp"

namespace rlbox {
/**
 * @brief Implementation of tainted_volatile data wrapper for pointer data. This
 * wrapper indicates that this data is located in memory that can be modified by
 * the sandbox.
 *
 * @details tainted_volatile_pointer data in addition to being untrusted (since
 * it is memory written to be sandboxed code), should be carefully checked for
 * double-read or time-of-check-time-of-use style bugs, i.e., if this data is
 * read twice, the data could have been changed asynchronously by a concurrent
 * sandbox thread. To safely handle tainted_volatile_pointer data, the host
 * application should make a copy of tainted_volatile_pointer data into the
 * application memory (tainted data) or call copy_and_verify to make a sanitized
 * copy of this data.
 *
 * @tparam TAppRep is the type of the data being wrapped.
 * @tparam TSbx is the type of the sandbox plugin that represents the underlying
 * sandbox implementation.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_impl<
    TUseAppRep, TAppRep, TSbx,
    RLBOX_SPECIALIZE(
        !TUseAppRep && std::is_pointer_v<TAppRep> &&
        TSbx::mTaintedVolatilePointerChoice ==
            tainted_volatile_pointer_t::TAINTED_VOLATILE_POINTER_STANDARD)>
    : public tainted_base<TUseAppRep, TAppRep, TSbx> {
  KEEP_RLBOX_CLASSES_FRIENDLY;

 protected:
  /**
   * @brief The sandbox representation of data for this wrapper
   */
  using TSbxRep = detail::rlbox_base_types_convertor<TAppRep, TSbx>;

  /**
   * @brief The current class's type
   */
  using this_t = tainted_impl<
      TUseAppRep, TAppRep, TSbx,
      RLBOX_SPECIALIZE(
          !TUseAppRep && std::is_pointer_v<TAppRep> &&
          TSbx::mTaintedVolatilePointerChoice ==
              tainted_volatile_pointer_t::TAINTED_VOLATILE_POINTER_STANDARD)>;

  void dummy_check() {
    static_assert(
        std::is_same_v<this_t, std::remove_pointer_t<decltype(this)>>);
  }

  detail::tainted_rep_t<TSbxRep> data{0};

 public:
  /**
   * @brief Construct a new tainted volatile_pointer set to null
   */
  inline tainted_impl() = default;
  /**
   * @brief Copy constructor: Construct a new tainted volatile_pointer object
   */
  inline tainted_impl(const this_t&) = default;
  /**
   * @brief Move constructor: Construct a new tainted volatile_pointer object
   */
  inline tainted_impl(this_t&&) noexcept = default;

  /**
   * @brief Construct a new tainted volatile_pointer with a nullptr
   * @param aNull is a nullptr
   */
  inline tainted_impl([[maybe_unused]] const std::nullptr_t& aNull) : data(0) {}

  /**
   * @brief Construct a new tainted object from another tainted wrapper
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the rhs AppRep
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam TExtraOther... is the extra args of the rhs
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the constructible criterion
   * @param aOther is the rhs being assigned
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>> &&
          !detail::is_same_wrapper_type_v<this_t, TWrap, TUseAppRepOther,
                                          TAppRepOther, TSbx, TExtraOther...> &&
          std::is_constructible_v<detail::tainted_rep_t<TAppRep>,
                                  TAppRepOther>)>
  inline tainted_impl(
      const TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>& aOther)
      : data(aOther.raw_sandbox_rep()) {}

  /**
   * @brief Destroy the tainted volatile_pointer object
   */
  inline ~tainted_impl() = default;

  ////////////////////////////////

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified()
      const {
    /// We need to construct an example_unsandboxed_ptr in order to call @ref
    /// rlbox::rlbox_sandbox::get_unsandboxed_pointer_with_example. We use a
    /// cool trick to construct such an example. Since tainted_volatile is the
    /// type of data in sandbox memory, the address of data (&data) itself
    /// refers to a location in sandbox memory and can thus be the
    /// example_unsandboxed_ptr. See Appendix A of
    /// https://arxiv.org/pdf/2003.00572.pdf for more details.
    const void* example_unsandboxed_ptr = &data;
    auto raw_ptr =
        rlbox_sandbox<TSbx>::template get_unsandboxed_pointer_with_example<
            TAppRep>(data, example_unsandboxed_ptr);
    return raw_ptr;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TAppRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TAppRep> UNSAFE_unverified(
      rlbox_sandbox<TSbx>& aSandbox) const {
    return aSandbox.get_unsandboxed_pointer(data);
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data in the sandboxed
   * ABI.
   * @return detail::tainted_rep_t<TSbxRep> is the raw data
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed() const {
    return data;
  }

  /**
   * @brief Unsafely remove the tainting and get the raw data converted to the
   * sandboxed ABI.
   * @param aSandbox is the sandbox this tainted value belongs to
   * @return detail::tainted_rep_t<TSbxRep> is the raw data in the sandboxed ABI
   */
  [[nodiscard]] inline detail::tainted_rep_t<TSbxRep> UNSAFE_sandboxed(
      [[maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const {
    return UNSAFE_sandboxed();
  }

  ////////////////////////////////

  /**
   * @brief Copy assignment operator
   * @param aOther is the rhs argument
   * @return this_t&
   * returns this object
   */
  inline this_t& operator=(const this_t& aOther) noexcept = default;

  /**
   * @brief Move assignment operator
   * @param aOther is the rhs argument
   * @return this_t&
   * returns this object
   */
  inline this_t& operator=(this_t&& aOther) noexcept = default;

  /**
   * @brief Operator= for tainted values from another tainted wrapper
   * @tparam TWrap is the rhs wrapper type
   * @tparam TUseAppRepOther is the rhs AppRep
   * @tparam TAppRepOther is the type of the rhs value being wrapped
   * @tparam TExtraOther... is the extra args of the rhs
   * @tparam RLBOX_REQUIRE param checks to see if this (1) won't be handled the
   * original class's copy/move constructor (2) is a tainted wrapper and (3)
   * meets the assignable criterion
   * @param aOther is the rhs being assigned
   * @return this_t& is the reference
   * to this value
   */
  template <
      template <bool, typename, typename, typename...> typename TWrap,
      bool TUseAppRepOther, typename TAppRepOther, typename... TExtraOther,
      RLBOX_REQUIRE(
          detail::is_tainted_any_wrapper_v<
              TWrap<TUseAppRepOther, TAppRepOther, TSbx, TExtraOther...>> &&
          !detail::is_same_wrapper_type_v<this_t, TWrap, TUseAppRepOther,
                                          TAppRepOther, TSbx, TExtraOther...> &&
          std::is_assignable_v<detail::tainted_rep_t<TAppRep>&, TAppRepOther>)>
  inline this_t& operator=(const TWrap<TUseAppRepOther, TAppRepOther, TSbx,
                                       TExtraOther...>& aOther) {
    data = aOther.raw_sandbox_rep();
    return *this;
  }

  /**
   * @brief Operator= for tainted values with a nullptr
   * @param aNull is a nullptr
   */
  inline this_t& operator=(
      [[maybe_unused]] const std::nullptr_t& aNull) noexcept {
    data = 0;
    return *this;
  }

  ////////////////////////////////

 protected:
  /**
   * @brief tainted_volatile type of the sandbox
   */
  template <typename TSub>
  using tainted_volatile = tainted_impl<false, TSub, TSbx>;

  /**
   * @brief Result type of operator*
   */
  using TOpDeref = tainted_volatile<std::remove_pointer_t<TAppRep>>;

 public:
  /**
   * @brief Operator* which dereferences tainted_volatile and gives another
   * tainted_volatile&
   * @return TOpDeref& is the reference to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref& operator*() const {
    auto raw_ptr = raw_host_rep();
    auto raw_ptr_cast = reinterpret_cast<TOpDeref*>(raw_ptr);
    return *raw_ptr_cast;
  }

  /**
   * @brief Operator* which dereferences tainted and gives a tainted_volatile&
   * @return TOpDeref* is the ptr to the sandbox memory that holds this
   * data, i.e., memory which is a tainted_volatile type
   */
  inline TOpDeref* operator->() const noexcept {
    // call the deference operator * and then take the address of the result
    // Use std::adress of so we don't call the operator overload of operator &
    return std::addressof(**this);
  }

 protected:
  using TOpAddrOf = tainted<std::add_pointer_t<TAppRep>, TSbx>;

 public:
  /**
   * @brief Operator& which gets the address of tainted_volatile to get tainted
   * @return TOpAddrOf is the pointer to the sandbox memory that holds this data
   */
  inline TOpAddrOf operator&() {
    // Deliberately use a C style cast as we we want to get rid of any CV
    // qualifers here. CV qualifiers are moved inside the wrapper type and thus
    // continue to be tracked.

    // NOLINTNEXTLINE(google-readability-casting)
    auto* data_ptr = (std::add_pointer_t<TAppRep>)&data;

    /// \todo Should not be unchecked
    auto ret = TOpAddrOf::from_unchecked_raw_pointer(data_ptr);
    return ret;
  }
};

}  // namespace rlbox