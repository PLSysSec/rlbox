/**
 * @file rlbox_stdint_types.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header declares types that serve as markers for the aliased
 * integer types such as `int32_t` or `size_t` in scenarios where we need to
 * distinguish this from other integer types like `int` or `long`. This is used
 * when we have to convert data from one ABI to another for example LP64 to
 * ILP32
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace rlbox {

namespace detail {
/**
 * @brief base type used for rlbox stdint types like `uint8_t`
 */
class rlbox_stdint_base_t {};
}  // namespace detail

/**
 * @brief rlbox type to represent `uint8_t`
 */
class rlbox_uint8_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = uint8_t;
};
/**
 * @brief rlbox type to represent `int8_t`
 */
class rlbox_int8_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = int8_t;
};
/**
 * @brief rlbox type to represent `uint16_t`
 */
class rlbox_uint16_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = uint16_t;
};
/**
 * @brief rlbox type to represent `int16_t`
 */
class rlbox_int16_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = int16_t;
};
/**
 * @brief rlbox type to represent `uint32_t`
 */
class rlbox_uint32_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = uint32_t;
};
/**
 * @brief rlbox type to represent `int32_t`
 */
class rlbox_int32_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = int32_t;
};
/**
 * @brief rlbox type to represent `uint64_t`
 */
class rlbox_uint64_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = uint64_t;
};
/**
 * @brief rlbox type to represent `int64_t`
 */
class rlbox_int64_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = int64_t;
};
/**
 * @brief rlbox type to represent `size_t`
 */
class rlbox_size_t : public detail::rlbox_stdint_base_t {
 public:
  using equivalent_type_t = size_t;
};

}  // namespace rlbox
