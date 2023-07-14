/**
 * @file rlbox_helpers.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc utilities and macros useful for rlbox's
 * implementation
 */
#pragma once

/**
 * @brief Macro to be used in other macros to force users of the macros to add a
 * semi colon.
 */
#define RLBOX_REQUIRE_SEMI_COLON static_assert(true)

/**
 * @brief Macro to be used in specialization of different classes
 */
#define RLBOX_SPECIALIZE(...) typename std::enable_if_t<__VA_ARGS__>

/**
 * @brief Macro to be used to specify argument restrictions as template
 * condition
 */
#define RLBOX_REQUIRE(...) std::enable_if_t<__VA_ARGS__>* = nullptr

/**
 * @brief For internal use only. Make sure RLBox's various tainted classes can
 * access the private members of other tainted classes.
 */
#define KEEP_RLBOX_CLASSES_FRIENDLY               \
  template <typename TU1>                         \
  friend class rlbox_sandbox;                     \
                                                  \
  template <bool TU1, typename TU2, typename TU3> \
  friend class tainted_fundamental_or_enum;       \
                                                  \
  template <bool TU1, typename TU2, typename TU3> \
  friend class tainted_fixed_aligned_pointer;     \
                                                  \
  template <bool TU1, typename TU2, typename TU3> \
  friend class tainted_relocatable_pointer;       \
                                                  \
  template <bool TU1, typename TU2, typename TU3> \
  friend class tainted_volatile_standard_pointer; \
  RLBOX_REQUIRE_SEMI_COLON

/**
 * @brief For internal use only. Within RLBox's code, there is some use of
 * `UNSAFE_unverified` which are necessary and safe. We use the below name in
 * those places to keep the code readable.
 */
#define raw_host_rep UNSAFE_unverified

/**
 * @brief For internal use only. Within RLBox's code, there is some use of
 * `UNSAFE_sandboxed` which are necessary and safe. We use the below name in
 * those places to keep the code readable.
 */
#define raw_sandbox_rep UNSAFE_sandboxed
