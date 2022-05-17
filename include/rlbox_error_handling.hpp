#pragma once

/**
 * @file rlbox_error_handling.hpp
 * @brief This file contains common error utilies and strings used in the rlbox
 * library to handle errors and generate helpful error messages.
 */

// Sometimes needed depending on configuration of error handling
#include <cstdlib>   // IWYU pragma: keep
#include <iostream>  // IWYU pragma: keep

/**
 * @brief Used as part of error messages to refer to the tutorials
 */
#define RLBOX_REFER_DOCS_MESSAGE \
  "For more details, see examples provided in https://docs.rlbox.dev"

/**
 * @brief Used as part of error messages when rlbox gets to an unexpected state
 * or to unimplemented code
 */
#define RLBOX_FILE_BUG_MESSAGE \
  "Please file a bug at https://github.com/PLSysSec/rlbox"

namespace rlbox::detail {

/**
 * @brief An internal RLBox function that is used to perform runtime assertion
 * checks. This is used in the RLBox API to check a variety of RLBox invariants.
 * On success, this function does nothing.
 * If the check fails, then the function calls `abort`.
 * The check failure can be customized to call a custom abort handler or to
 * throw exceptions instead. See examples for details on how to customize.
 *
 * @details
 * Usage of this function is as follows. This would abort if the check fails.
 *
 * @code
 * dynamic_check(value == 5, "Unexpected value calls 5");
 * @endcode
 *
 * You can customize RLBox to call a custom abort handler on failure as shown
 * below
 * @code
 * void custom_abort(const char* msg) {
 *   //...
 * }
 * #define RLBOX_CUSTOM_ABORT(msg) custom_abort(msg)
 * @endcode
 *
 * Alternately you can customize RLBox to throw an exception on failure as
 * shown below
 * @code
 * #define RLBOX_USE_EXCEPTIONS_ON_ERROR
 * @endcode
 * @param aCheckSucceeded is the result of a boolean runtime check
 * @param aMsg is the error message to display on error
 */
inline void dynamic_check(bool aCheckSucceeded, const char* aMsg) {
#if defined(RLBOX_USE_EXCEPTIONS_ON_ERROR) && defined(RLBOX_CUSTOM_ABORT)
#  error \
      "You can define only one of the two macros RLBOX_USE_EXCEPTIONS_ON_ERROR and RLBOX_CUSTOM_ABORT"
#endif
  if (!aCheckSucceeded) {
#if __cpp_exceptions && defined(RLBOX_USE_EXCEPTIONS_ON_ERROR)
    throw std::runtime_error(aMsg);
#else
#  ifdef RLBOX_CUSTOM_ABORT
    RLBOX_CUSTOM_ABORT(aMsg);
#  else
    std::cerr << aMsg << std::endl;
    std::abort();
#  endif
#endif
  }
}

}  // namespace rlbox::detail