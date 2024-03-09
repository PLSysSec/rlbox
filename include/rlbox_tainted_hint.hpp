/**
 * @file rlbox_tainted_hint.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the tainted_boolean_hint class which is a type
 * that serves as a "hint" and not a definite answer.
 */

#pragma once

#include <cstddef>
#include <stdint.h>
// IWYU doesn't seem to recognize the call to std::is_same_v so force IWYU to
// keep the next include
#include <type_traits>  // IWYU pragma: keep

#include "rlbox_error_handling.hpp"
#include "rlbox_tainted_base.hpp"
#include "rlbox_types.hpp"

namespace rlbox {

/**
 * @brief Tainted boolean value that serves as a "hint" and not a definite
 * answer. Comparisons with a tainted_volatile return such hints. They are not
 * `tainted<bool>` values because a compromised sandbox can modify
 * tainted_volatile data at any time.
 */
template <bool TUseAppRep, typename TAppRep, typename TSbx>
class tainted_boolean_hint : public tainted_base<TUseAppRep, TAppRep, TSbx> {
 protected:
  static_assert(std::is_same_v<TAppRep, bool>);

  /**
   * @brief The current class's type
   */
  using this_t = rlbox::tainted_boolean_hint<TUseAppRep, TAppRep, TSbx>;

  bool val{false};

 public:
  /**
   * @brief Construct a new tainted_boolean_hint object
   */
  inline tainted_boolean_hint() = default;

  /**
   * @brief Construct a new tainted_boolean_hint object with an initial value
   */
  inline tainted_boolean_hint(bool aVal) : val(aVal) {}

  /**
   * @brief Copy constructor: Construct a new tainted_boolean_hint object
   */
  inline tainted_boolean_hint(const this_t&) = default;

  /**
   * @brief Move constructor: Construct a new tainted_boolean_hint object
   */
  inline tainted_boolean_hint(this_t&&) noexcept = default;

  /**
   * @brief Destroy the tainted_boolean_hint object
   */
  inline ~tainted_boolean_hint() = default;

  /**
   * @brief Copy assignment operator
   * @param aOther is the rhs argument
   * @return this_t& returns
   * this object
   */
  inline this_t& operator=(const this_t& aOther) noexcept = default;

  /**
   * @brief Move assignment operator
   * @param aOther is the rhs argument
   * @return this_t& returns
   * this object
   */
  inline this_t& operator=(this_t&& aOther) noexcept = default;

  /**
   * @brief Operator= for boolean values
   * @param aOther is the rhs being assigned
   * @return tainted_boolean_hint& is the reference to this value
   */
  inline tainted_boolean_hint& operator=(bool aOther) noexcept {
    val = aOther;
    return *this;
  }

  /**
   * @brief Operator! that negates the current hint
   * @return tainted_boolean_hint with the negation of this value
   */
  inline tainted_boolean_hint operator!() const noexcept {
    return tainted_boolean_hint(!val);
  }

  /**
   * @brief This function returns the hint as a normal boolean
   * @tparam TN is the size char* that is provided as the reason
   * @param aReason is the reason provided by the developer explaining why
   * converting this hint to a boolean is safe. This reason string is not used
   * for any purpose, but serves as documentation for the future as to why this
   * conversion is safe.
   * @return bool which is the hint converted to a normal boolean
   */
  template <size_t TN>
  inline bool unverified_safe_because(
      [[maybe_unused]] const char (&aReason)[TN]) const noexcept {
    return val;
  }

  /**
   * @brief Return the hint converted to a normal boolean. This is unsafe as the
   * application is not checking if this conversion is safe to perform. Use of
   * unsafe operations may lead to confused deputy attacks.
   * @return bool which is the hint converted to a normal boolean
   */
  [[nodiscard]] inline bool UNSAFE_unverified() const noexcept { return val; }

  /**
   * @brief Return the hint converted to a normal boolean. This is the same as
   * the function without the sandbox parameter. This overload exists only for
   * consistency with other tainted wrappers. This is unsafe as the application
   * is not checking if this conversion is safe to perform. Use of unsafe
   * operations may lead to confused deputy attacks.
   * @param aSandbox is the sandbox whose memory was target of a comparison to
   * generate this hint
   * @return bool which is the hint converted to a normal boolean
   */
  [[nodiscard]] inline bool UNSAFE_unverified(
      [[maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const noexcept {
    return UNSAFE_unverified();
  }

  /**
   * @brief Return the hint converted to a sandboxed boolean in the form of an
   * int. This is unsafe as the application is not checking if this conversion
   * is safe to perform. Use of unsafe operations may lead to confused deputy
   * attacks.
   * @return uint8_t which is the hint converted to a sandboxed boolean in the
   * form of an int
   */
  [[nodiscard]] inline uint8_t UNSAFE_sandboxed() const noexcept {
    return val ? 1 : 0;
  }

  /**
   * @brief Return the hint converted to a sandboxed boolean in the form of an
   * int. This is the same as the function without the sandbox parameter. This
   * overload exists only for consistency with other tainted wrappers. This is
   * unsafe as the application is not checking if this conversion is safe to
   * perform. Use of unsafe operations may lead to confused deputy attacks.
   * @return uint8_t which is the hint converted to a sandboxed boolean in the
   * form of an int
   */
  [[nodiscard]] inline uint8_t UNSAFE_sandboxed(
      [[maybe_unused]] rlbox_sandbox<TSbx>& aSandbox) const noexcept {
    return UNSAFE_sandboxed();
  }

  /**
   * @brief This function exists for consistency with the other tainted types.
   * This function will create a compile time error if called. This is because
   * there is no sensible way to verify a boolean hint. The caller must instead
   * use `unverified_safe_because` and modify the application code to be safe
   * even if the hint is incorrect.
   * @tparam TDummy parameter exists to make sure the assert only fires when
   * called
   */
  template <typename TDummy = void>
  inline bool copy_and_verify(...) const {
    rlbox_static_fail(
        TAppRep,
        "You can't call copy_and_verify on this value, as this is a result of "
        "a comparison with memory accessible by the sandbox. \n"
        "The sandbox could unexpectedly change the value leading to "
        "time-of-check-time-of-use attacks. \n"
        "You can avoid this by making a local copy of the data. For example, "
        "if your original code, looked like \n"
        "if ((tainted_ptr->member == 5).copy_and_verify(...)) { ... } \n\n"
        "Change this to \n\n"
        "tainted<int> val = tainted_ptr->member\n"
        "if ((val == 5).copy_and_verify(...)) { ... } \n\n"
        "tainted<int, T_Sbx> foo(rlbox_sandbox<T_Sbx>& sandbox) {...} \n\n"
        "Alternately, if you are sure your code is safe you can use the "
        "unverified_safe_because API to remove tainting\n");

    // this is never executed, but we need it for the function to type-check
    return false;
  }
};

}  // namespace rlbox
