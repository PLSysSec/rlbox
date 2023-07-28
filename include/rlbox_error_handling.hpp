/**
 * @file rlbox_error_handling.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This file contains common error utilies and strings used in the rlbox
 * library to handle errors and generate helpful error messages.
 */

#pragma once

// Sometimes needed depending on configuration of error handling
#include <cstdlib>   // IWYU pragma: keep
#include <iostream>  // IWYU pragma: keep

/**
 * @brief Used as part of error messages to refer to the tutorials
 */
#define RLBOX_REFER_DOCS_MESSAGE \
  "For more details, see examples provided in https://rlbox.dev"

/**
 * @brief Used as part of error messages when RLBox gets to an unexpected state
 * or to unimplemented code
 */
#define RLBOX_FILE_BUG_MESSAGE \
  "Please file a bug at https://github.com/PLSysSec/rlbox"

/**
 * @brief Used to indicate an RLBox feature which is incomplete.
 */
#define RLBOX_NOT_IMPLEMENTED_MESSAGE \
  "This features of RLBox is not yet implemented. " RLBOX_FILE_BUG_MESSAGE

namespace rlbox::detail {

/**
 * @brief An internal RLBox function that is called when a runtime error occurs.
 * The function typically will call `abort`. The function can be customized
 * to call a custom abort handler or to throw exceptions instead. See examples
 * for details on how to customize.
 *
 * @details
 * Usage of this function is as follows. This would abort if the check fails.
 *
 * @code {.cpp} error_occured("Unexpected error");
 * @endcode
 *
 * You can customize RLBox to call a custom abort handler on failure as shown
 * below
 * @code {.cpp} void custom_abort(const char* msg) {
 *   //...
 * }
 * #define RLBOX_CUSTOM_ABORT(msg) custom_abort(msg)
 * @endcode
 *
 * Alternately you can customize RLBox to throw an exception on failure as shown
 * below
 * @code {.cpp} #define RLBOX_USE_EXCEPTIONS_ON_ERROR
 * @endcode
 * @param aMsg is the error message to display on error
 */
[[noreturn]] inline void error_occured(const char* aMsg)
#ifndef RLBOX_USE_EXCEPTIONS_ON_ERROR
    noexcept
#endif
{
#if defined(RLBOX_USE_EXCEPTIONS_ON_ERROR) && defined(RLBOX_CUSTOM_ABORT)
#  error \
      "You can define only one of the two macros RLBOX_USE_EXCEPTIONS_ON_ERROR and RLBOX_CUSTOM_ABORT"
#endif
#if __cpp_exceptions && defined(RLBOX_USE_EXCEPTIONS_ON_ERROR)
  throw std::runtime_error(aMsg);
#else
#  ifdef RLBOX_CUSTOM_ABORT
  RLBOX_CUSTOM_ABORT(aMsg);
  std::abort();
#  else
  std::cerr << aMsg << std::endl;
  std::abort();
#  endif
#endif
}

/**
 * @brief An internal RLBox function that is used to perform runtime assertion
 * checks. This is used in the RLBox API to check a variety of RLBox invariants.
 * On success, this function does nothing.
 * If the check fails, then the function calls @ref
 * rlbox::detail::error_occured.
 *
 * @details
 * Usage of this function is as follows. This would abort if the check fails.
 *
 * @code {.cpp}
 * dynamic_check(value == 5, "Unexpected value calls 5");
 * @endcode
 *
 * @param aCheckSucceeded is the result of a boolean runtime check
 * @param aMsg is the error message to display on error
 */
inline void dynamic_check(bool aCheckSucceeded, const char* aMsg)
#ifndef RLBOX_USE_EXCEPTIONS_ON_ERROR
    noexcept
#endif
{
  if (!aCheckSucceeded) {
    error_occured(aMsg);
  }
}

#ifdef RLBOX_REPLACE_COMPILE_CHECKS_WITH_RUNTIME_ERRORS
#  define rlbox_static_assert(...) rlbox::detail::dynamic_check(__VA_ARGS__)
#else
#  define rlbox_static_assert(...) static_assert(__VA_ARGS__)
#endif

namespace fail_detail {
template <typename...>
struct helper_struct {
  static constexpr bool mVal = false;
};

struct dummy_struct {};

template <>
struct helper_struct<dummy_struct> {
  static constexpr bool mVal = true;
};

}  // namespace fail_detail

#define rlbox_static_fail(T, msg) \
  rlbox_static_assert(rlbox::detail::fail_detail::helper_struct<T>::mVal, msg)

}  // namespace rlbox::detail