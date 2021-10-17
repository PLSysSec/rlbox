#pragma once
// IWYU pragma: private, include "rlbox.hpp"
// IWYU pragma: friend "rlbox_.*\.hpp"

namespace rlbox {

template<typename T, typename T_Sbx>
class tainted_opaque
{
private:
  T data{ 0 };

public:
  template<typename T2 = T>
  void set_zero()
  {
    data = 0;
  }
};

template<typename T, typename T_Sbx>
class tainted;

template<typename T, typename T_Sbx>
class tainted_volatile;

/**
 * @brief Tainted boolean value that serves as a "hint" and not a definite
 * answer.  Comparisons with a tainted_volatile return such hints.  They are
 * not `tainted<bool>` values because a compromised sandbox can modify
 * tainted_volatile data at any time.
 */
class tainted_boolean_hint
{
private:
  bool val;

public:
  tainted_boolean_hint(bool init)
    : val(init)
  {}
  tainted_boolean_hint(const tainted_boolean_hint&) = default;
  inline tainted_boolean_hint& operator=(bool rhs)
  {
    val = rhs;
    return *this;
  }
  inline tainted_boolean_hint operator!() { return tainted_boolean_hint(!val); }
  template<size_t N>
  inline bool unverified_safe_because(const char (&reason)[N]) const
  {
    (void)reason; /* unused */
    return val;
  }
  inline bool UNSAFE_unverified() const { return val; }
  inline bool UNSAFE_unverified() { return val; }
  inline auto INTERNAL_unverified_safe() { return UNSAFE_unverified(); }
  inline auto INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  // Add a template parameter to make sure the assert only fires when called
  template<typename T=void>
  inline bool copy_and_verify(...) const
  {
    rlbox_detail_static_fail_because(
      detail::true_v<T>,
      "You can't call copy_and_verify on this value, as this is a result of a "
      "comparison with memory accessible by the sandbox. \n"
      "The sandbox could unexpectedly change the value leading to "
      "time-of-check-time-of-use attacks. \n"
      "You can avoid this by making a local copy of the data."
      "For example, if your original code, looked like \n"
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

/**
 * @brief Tainted integer value that serves as a "hint" and not a definite
 * answer.  Comparisons with a tainted_volatile return such hints.  They are
 * not `tainted<int>` values because a compromised sandbox can modify
 * tainted_volatile data at any time.
 */
class tainted_int_hint
{
private:
  int val;

public:
  tainted_int_hint(int init)
    : val(init)
  {}
  tainted_int_hint(const tainted_int_hint&) = default;
  inline tainted_int_hint& operator=(int rhs)
  {
    val = rhs;
    return *this;
  }
  inline tainted_boolean_hint operator!() { return tainted_boolean_hint(!val); }
  template<size_t N>
  inline int unverified_safe_because(const char (&reason)[N]) const
  {
    (void)reason; /* unused */
    return val;
  }
  inline int UNSAFE_unverified() const { return val; }
  inline int UNSAFE_unverified() { return val; }
  inline auto INTERNAL_unverified_safe() { return UNSAFE_unverified(); }
  inline auto INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  // Add a template parameter to make sure the assert only fires when called
  template<typename T=void>
  inline int copy_and_verify(...) const
  {
    rlbox_detail_static_fail_because(
      detail::true_v<T>,
      "You can't call copy_and_verify on this value, as this is a result of a "
      "comparison with memory accessible by the sandbox. \n"
      "The sandbox could unexpectedly change the value leading to "
      "time-of-check-time-of-use attacks. \n"
      "You can avoid this by making a local copy of the data."
      "For example, if your original code, looked like \n"
      "if ((tainted_ptr->member == 5).copy_and_verify(...)) { ... } \n\n"
      "Change this to \n\n"
      "tainted<int> val = tainted_ptr->member\n"
      "if ((val == 5).copy_and_verify(...)) { ... } \n\n"
      "tainted<int, T_Sbx> foo(rlbox_sandbox<T_Sbx>& sandbox) {...} \n\n"
      "Alternately, if you are sure your code is safe you can use the "
      "unverified_safe_because API to remove tainting\n");

    // this is never executed, but we need it for the function to type-check
    return 0;
  }
};

template<typename T_Sbx>
class rlbox_sandbox;

template<typename T, typename T_Sbx>
class sandbox_callback;

template<typename T, typename T_Sbx>
class app_pointer;

class rlbox_noop_sandbox;

class rlbox_dylib_sandbox;
}
