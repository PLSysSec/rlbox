/**
 * @file rlbox_forwarder.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements a header-macro (a header that can be included
 * multiple times, has parameters that can be set prior to being included.
 * Header-macros are easier to debug in gdb than standard macros) that forwards
 * all C++ operators to a specified base class or a member function. This header
 * itself is formatted such that it parses as a valid C++ file to ensure editor
 * intellisense works.
 */

#if !defined(RLBOX_FORWARD_TARGET_CLASS) &&                                    \
    !defined(RLBOX_FORWARD_CURR_CLASS) && !defined(RLBOX_FORWARD_TO_OBJECT) && \
    !defined(RLBOX_FORWARD_TO_SUBCLASS)
// If no parameters are defined, assume that the file is being parsed for
// intellisense.
#  define RLBOX_FORWARD_SIMULATE_FOR_INTELLISENSE
#elif !(                                   \
    defined(RLBOX_FORWARD_TARGET_CLASS) && \
    defined(RLBOX_FORWARD_CURR_CLASS) &&   \
    (defined(RLBOX_FORWARD_TO_OBJECT) || defined(RLBOX_FORWARD_TO_SUBCLASS)))
// If only some parameters are not defined, report an error
#  error \
      "RLBOX_FORWARD: define RLBOX_FORWARD_TARGET_CLASS, RLBOX_FORWARD_CURR_CLASS, and one of RLBOX_FORWARD_TO_OBJECT and RLBOX_FORWARD_TO_SUBCLASS"
#endif

#ifdef RLBOX_FORWARD_SIMULATE_FOR_INTELLISENSE

#  include <type_traits>
#  include <utility>
#  include <optional>

#  include "rlbox_helpers.hpp"
#  include "rlbox_type_traits.hpp"

namespace rlbox {

class rlbox_forwarder;

#  define RLBOX_FORWARD_CURR_CLASS rlbox_forwarder
#  define RLBOX_FORWARD_TARGET_CLASS std::optional<int>
#  define RLBOX_FORWARD_TO_SUBCLASS
// no definition for RLBOX_FORWARD_TO_OBJECT

class rlbox_forwarder : std::optional<int> {
 public:
#endif

#ifdef RLBOX_FORWARD_TO_SUBCLASS
#  define RLBOX_FORWARD_CONSTRUCTOR_NAME RLBOX_FORWARD_TARGET_CLASS
// Expression that casts this to its base class while accounting for any const
// volatile qualifiers
#  define RLBOX_FORWARD_TARGET_EXPR                                           \
    (*static_cast<                                                            \
        rlbox::detail::copy_cvref_t<std::remove_reference_t<decltype(*this)>, \
                                    RLBOX_FORWARD_TARGET_CLASS>*>(this))
#else
#  define RLBOX_FORWARD_CONSTRUCTOR_NAME RLBOX_FORWARD_TO_OBJECT
#  define RLBOX_FORWARD_TARGET_EXPR RLBOX_FORWARD_TO_OBJECT
#endif

  inline RLBOX_FORWARD_CURR_CLASS() = default;
  inline RLBOX_FORWARD_CURR_CLASS(const RLBOX_FORWARD_CURR_CLASS&) = default;
  inline RLBOX_FORWARD_CURR_CLASS(RLBOX_FORWARD_CURR_CLASS&&) noexcept(
      std::is_nothrow_move_constructible_v<RLBOX_FORWARD_TARGET_CLASS>) =
      default;

  // Single parameter constructors need to handle explicit We could use
  // explicit(bool), i.e., explicit(std::is_convertible_v<TArg,
  // RLBOX_FORWARD_TARGET_CLASS>) in C++ 20, but need to do this manually in
  // C++17

  template <typename TArg,
            RLBOX_REQUIRE(
                std::is_constructible_v<RLBOX_FORWARD_TARGET_CLASS, TArg>&&
                    std::is_convertible_v<TArg, RLBOX_FORWARD_TARGET_CLASS>)>
  inline RLBOX_FORWARD_CURR_CLASS(TArg&& aArg)
      : RLBOX_FORWARD_CONSTRUCTOR_NAME(std::forward<TArg>(aArg)) {}

  template <
      typename TArg,
      RLBOX_REQUIRE(std::is_constructible_v<RLBOX_FORWARD_TARGET_CLASS, TArg> &&
                    !std::is_convertible_v<TArg, RLBOX_FORWARD_TARGET_CLASS>)>
  explicit inline RLBOX_FORWARD_CURR_CLASS(TArg&& aArg)
      : RLBOX_FORWARD_CONSTRUCTOR_NAME(std::forward<TArg>(aArg)) {}

  // Multi parameter constructors are not explicit by default
  template <typename... TArgs,
            RLBOX_REQUIRE(
                std::is_constructible_v<RLBOX_FORWARD_TARGET_CLASS, TArgs...> &&
                (sizeof...(TArgs) > 1))>
  inline RLBOX_FORWARD_CURR_CLASS(TArgs&&... aArgs)
      : RLBOX_FORWARD_CONSTRUCTOR_NAME(std::forward<TArgs>(aArgs)...) {}

  inline ~RLBOX_FORWARD_CURR_CLASS() = default;

  inline RLBOX_FORWARD_CURR_CLASS& operator=(
      const RLBOX_FORWARD_CURR_CLASS& aArg) noexcept = default;
  inline RLBOX_FORWARD_CURR_CLASS& operator=(
      RLBOX_FORWARD_CURR_CLASS&& aOther) noexcept = default;
  template <typename TArg>
  inline auto& operator=(TArg&& aArg) {
    RLBOX_FORWARD_TARGET_EXPR = std::forward<TArg>(aArg);
    return *this;
  }

#ifdef RLBOX_FORWARD_TO_SUBCLASS
  /**
   * @brief Operator== forwarding
   *
   * @tparam TArg is the type of the rhs
   * @param aArg is the rhs of the equals check
   * @return the result of operator==
   */
  template <typename TArg>
  inline auto operator==(TArg&& aArg) const
      noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR ==
                        std::forward<TArg>(aArg))) {
    return RLBOX_FORWARD_TARGET_EXPR == std::forward<TArg>(aArg);
  }

  /**
   * @brief Operator== specialization for the target class. This is necessary as
   * the above general version would result in an ambigous overload set if the
   * target class also implements equals with a generic equals operator.
   *
   * @param aArg Object of type RLBOX_FORWARD_TARGET_CLASS
   * @return result of equals comparison
   */
  // template <typename TDummy = int>
  inline auto operator==(const RLBOX_FORWARD_TARGET_CLASS& aArg) const
      noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR == aArg)) {
    return RLBOX_FORWARD_TARGET_EXPR == aArg;
  }
#else

/**
 * @brief Operator== forwarding when this wrapper is rhs
 *
 * @param aArg is the rhs of the equals check
 * @return the result of operator==
 */
inline auto operator==(const RLBOX_FORWARD_CURR_CLASS& aArg) const
    noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR ==
                      aArg.RLBOX_FORWARD_TO_OBJECT)) {
  return RLBOX_FORWARD_TARGET_EXPR == aArg.RLBOX_FORWARD_TO_OBJECT;
}

/**
 * @brief Operator== forwarding when the wrapped class is lhs
 *
 * @param aLhs is the lhs of the equals check of the wrapped type
 * @param aRhs is the rhs of the equals check of the wrapper type
 * @return the result of operator==
 */
friend inline bool operator==(const RLBOX_FORWARD_TARGET_CLASS& aLhs,
                              const RLBOX_FORWARD_CURR_CLASS& aRhs)
    // can't apply noexcept to an rhs object since the class hasn't finished its
    // definition
    noexcept(noexcept(aLhs == std::declval<RLBOX_FORWARD_TARGET_CLASS>())) {
  return aLhs == aRhs.RLBOX_FORWARD_TO_OBJECT;
}

#endif

#ifdef RLBOX_FORWARD_TO_SUBCLASS
  /**
   * @brief Operator!= forwarding
   *
   * @tparam TArg is the type of the rhs
   * @param aArg is the rhs of the not-equals check
   * @return the result of operator!=
   */
  template <typename TArg>
  inline auto operator!=(TArg&& aArg) const
      noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR !=
                        std::forward<TArg>(aArg))) {
    return RLBOX_FORWARD_TARGET_EXPR != std::forward<TArg>(aArg);
  }

  /**
   * @brief Operator!= specialization for the target class. This is necessary as
   * the above general version would result in an ambigous overload set if the
   * target class also implements not-equals with a generic not-equals operator.
   *
   * @param aArg Object of type RLBOX_FORWARD_TARGET_CLASS
   * @return result of not-equals comparison
   */
  // template <typename TDummy = int>
  inline auto operator!=(const RLBOX_FORWARD_TARGET_CLASS& aArg) const
      noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR != aArg)) {
    return RLBOX_FORWARD_TARGET_EXPR != aArg;
  }
#else

/**
 * @brief Operator!= forwarding when this wrapper is rhs
 *
 * @param aArg is the rhs of the not-equals check
 * @return the result of operator!=
 */
inline auto operator!=(const RLBOX_FORWARD_CURR_CLASS& aArg) const
    noexcept(noexcept(RLBOX_FORWARD_TARGET_EXPR !=
                      aArg.RLBOX_FORWARD_TO_OBJECT)) {
  return RLBOX_FORWARD_TARGET_EXPR != aArg.RLBOX_FORWARD_TO_OBJECT;
}

/**
 * @brief Operator!= forwarding when the wrapped class is lhs
 *
 * @param aLhs is the lhs of the not-equals check of the wrapped type
 * @param aRhs is the rhs of the not-equals check of the wrapper type
 * @return the result of operator!=
 */
friend inline bool operator!=(const RLBOX_FORWARD_TARGET_CLASS& aLhs,
                              const RLBOX_FORWARD_CURR_CLASS& aRhs)
    // can't apply noexcept to an rhs object since the class hasn't finished its
    // definition
    noexcept(noexcept(aLhs != std::declval<RLBOX_FORWARD_TARGET_CLASS>())) {
  return aLhs != aRhs.RLBOX_FORWARD_TO_OBJECT;
}

#endif

  template <typename TDummy = int>
  inline auto& operator*() const {
    return *RLBOX_FORWARD_TARGET_EXPR;
  }

  template <typename TDummy = int>
  explicit inline operator bool() const
      noexcept(noexcept(static_cast<bool>(RLBOX_FORWARD_TARGET_EXPR))) {
    return static_cast<bool>(RLBOX_FORWARD_TARGET_EXPR);
  }

#ifdef RLBOX_FORWARD_SIMULATE_FOR_INTELLISENSE
};

}  // namespace rlbox

#endif

#undef RLBOX_FORWARD_TARGET_EXPR
#undef RLBOX_FORWARD_CONSTRUCTOR_NAME
#undef RLBOX_FORWARD_SIMULATE_FOR_INTELLISENSE

#undef RLBOX_FORWARD_TARGET_CLASS
#undef RLBOX_FORWARD_CURR_CLASS
#undef RLBOX_FORWARD_TO_OBJECT
#undef RLBOX_FORWARD_TO_SUBCLASS
