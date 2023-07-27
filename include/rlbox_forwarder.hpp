/**
 * @file rlbox_forwarder.hpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief This header implements a header-macro (a header that can be included
 * multiple times, has parameters that can be set prior to being included.
 * Header-macros are easier to debug in gdb than standard macros) that forwards
 * all C++ operators to an underlying object. This header itself is formatted
 * such that it parses as a valid C++ file to ensure editor intellisense works.
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
  inline RLBOX_FORWARD_CURR_CLASS(RLBOX_FORWARD_CURR_CLASS&&) noexcept =
      default;
  template <typename... TArgs>
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

  template <typename TArg>
  inline auto operator==(TArg&& aArg) const noexcept {
    return RLBOX_FORWARD_TARGET_EXPR == std::forward<TArg>(aArg);
  }

  template <typename TArg>
  inline auto operator!=(TArg&& aArg) const noexcept {
    return RLBOX_FORWARD_TARGET_EXPR != std::forward<TArg>(aArg);
  }

  template <typename TDummy = int>
  inline auto& operator*() const {
    return *RLBOX_FORWARD_TARGET_EXPR;
  }

  template <typename TDummy = int>
  explicit inline operator bool() const noexcept {
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
