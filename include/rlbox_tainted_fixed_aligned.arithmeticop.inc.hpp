/**
 * @file rlbox_tainted_fixed_aligned.arithmeticop.inc.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements the arithmetic operators for the
 * tainted_fixed_aligned class. This file can be included multiple
 * times with after defining the macro RLBOX_ARITHMETIC_OP to an operator
 * like +, - etc.
 */

#ifndef RLBOX_ARITHMETIC_OP
// if RLBOX_ARITHMETIC_OP is not defined, we want this file to parse sensibly
// for when read on its own
#  define RLBOX_PARSE_FOR_STANDALONE
#endif

#ifdef RLBOX_PARSE_FOR_STANDALONE
#  define RLBOX_ARITHMETIC_OP +
#  define RLBOX_ARITHMETIC_ASSIGN_OP +=
#  include "rlbox_error_handling.hpp"
#  include "rlbox_helpers.hpp"
#  include "rlbox_types.hpp"
// IWYU doesn't seem to recognize the call to RLBOX_REQUIRE so force IWYU to
// keep the next include
#  include "rlbox_wrapper_traits.hpp"  // IWYU pragma: keep

#  include <stddef.h>

namespace rlbox {
template <bool TUseAppRep, typename TData, typename TSbx>
class dummy {
  using this_t = dummy<TUseAppRep, TData, TSbx>;
  int* data{0};
  inline bool is_null() noexcept { return data == nullptr; }
  static this_t from_unchecked_raw_pointer(int* aPtr) {
    this_t ret;
    ret.data = aPtr;
    return ret;
  }
#endif

  /**
   * @brief Operator which modifies a tainted pointer by aInc
   * @param aInc is the increment amount
   * @return this_t is the incremented tainted pointer
   */
  inline this_t operator RLBOX_ARITHMETIC_OP(size_t aInc) const {
    detail::dynamic_check(!is_null(), "Arithmetic on a tainted null pointer");
    auto new_data = data RLBOX_ARITHMETIC_OP aInc;
    const bool in_bounds =
        rlbox_sandbox<TSbx>::is_pointer_in_sandbox_memory_with_example(new_data,
                                                                       data);
    detail::dynamic_check(in_bounds, "Pointer offset not in sandbox");

    auto ret = this_t::from_unchecked_raw_pointer(new_data);
    return ret;
  }

  /**
   * @brief Operator which modifies a tainted pointer by aInc
   * @tparam T is the type of the tainted index
   * @param aInc is the increment amount
   * @return this_t is the incremented tainted pointer
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t operator RLBOX_ARITHMETIC_OP(tainted<T, TSbx> aInc) const {
    return (*this)RLBOX_ARITHMETIC_OP aInc.raw_host_rep();
  }

#define RLBOX_CONCAT_HELPER2(a) a## =
#define RLBOX_CONCAT_HELPER(a) RLBOX_CONCAT_HELPER2(a)
#define RLBOX_ARITHMETIC_ASSIGN_OP RLBOX_CONCAT_HELPER(RLBOX_ARITHMETIC_OP)

  /**
   * @brief Operator which modifies a tainted pointer by aInc and sets the
   * pointer
   * @param aInc is the increment amount
   * @return this_t& returns this object after modification
   */
  inline this_t& operator RLBOX_ARITHMETIC_ASSIGN_OP(size_t aInc) {
    this_t new_ptr = *this RLBOX_ARITHMETIC_OP aInc;
    *this = new_ptr;
    return *this;
  }

  /**
   * @brief Operator which modifies a tainted pointer by aInc and sets the
   * pointer
   * @tparam T is the type of the tainted index
   * @param aInc is the increment amount
   * @return this_t& returns this object after modification
   */
  template <typename T, RLBOX_REQUIRE(std::is_assignable_v<size_t&, T>)>
  inline this_t& operator RLBOX_ARITHMETIC_ASSIGN_OP(tainted<T, TSbx> aInc) {
    (*this) RLBOX_ARITHMETIC_ASSIGN_OP aInc.raw_host_rep();
    return *this;
  }

#ifdef RLBOX_PARSE_FOR_STANDALONE
};
}
#endif

#undef RLBOX_CONCAT_HELPER2
#undef RLBOX_CONCAT_HELPER
#undef RLBOX_PARSE_FOR_STANDALONE
#undef RLBOX_ARITHMETIC_ASSIGN_OP
#undef RLBOX_ARITHMETIC_OP
