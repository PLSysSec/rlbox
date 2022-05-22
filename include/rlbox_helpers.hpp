/**
 * @file rlbox_helpers.hpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header contains misc utilities and macros useful for rlbox's
 * implementation
 */
#pragma once

#define RLBOX_UNUSED(...) (void)__VA_ARGS__

#define RLBOX_REQUIRE_SEMI_COLON static_assert(true)

#define RLBOX_SPECIALIZE(...) typename std::enable_if_t<__VA_ARGS__>

#define RLBOX_REQUIRE(...) std::enable_if_t<__VA_ARGS__>* = nullptr

/**
 * @brief For internal use only. Make sure RLBox's various tainted classes can
 * access the private memmbers of other tainted classes.
 */
#define KEEP_RLBOX_CLASSES_FRIENDLY                   \
  template <typename TU1, typename TU2, typename TU3> \
  friend class tainted_fixed_aligned;                 \
                                                      \
  template <typename TU1, typename TU2, typename TU3> \
  friend class tainted_relocatable;                   \
                                                      \
  template <typename TU1, typename TU2, typename TU3> \
  friend class tainted_volatile_standard;             \
  RLBOX_REQUIRE_SEMI_COLON
