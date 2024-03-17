/**
 * @file test_plugin_reflection.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that the tainted/tainted_volatile type is correctly overidden in
 * plugins to rlbox_sandbox
 * @details Two cases to test:
 * - the sandbox plugin leaves the tainted/tainted_volatile type as default
 * - the sandbox plugin overrides the tainted/tainted_volatile type
 */

#include "test_include.hpp"

#include "rlbox_sandbox_plugin_base.hpp"
#include "rlbox_types.hpp"

#include <type_traits>

namespace rlbox {

class rlbox_default_tainted_testsandbox
    : public rlbox_sandbox_plugin_base<rlbox_default_tainted_testsandbox> {};

class rlbox_custom_tainted_testsandbox
    : public rlbox_sandbox_plugin_base<rlbox_custom_tainted_testsandbox> {
 public:
  static const constexpr tainted_pointer_t mTaintedPointerChoice =
      tainted_pointer_t::TAINTED_POINTER_FIXED_ALIGNED;
};

}  // namespace rlbox

RLBOX_DEFINE_BASE_TYPES_FOR(libtest_default, rlbox_default_tainted_testsandbox);
#define libtest_default_sandbox_invoke noop_arena_sandbox_invoke

RLBOX_DEFINE_BASE_TYPES_FOR(libtest_custom, rlbox_custom_tainted_testsandbox);
#define libtest_custom_sandbox_invoke noop_arena_sandbox_invoke

TEST_CASE("Test plugin tainted reflection", "[rlbox plugin reflection]") {
  // Check that by default, sandboxes assume that the plugin uses
  // tainted_relocatable and tainted_volatile_standard.
  REQUIRE(std::is_same_v<
          tainted_libtest_default<int>,
          tainted_impl<true, int, rlbox_default_tainted_testsandbox>>);
  REQUIRE(std::is_same_v<
          tainted_volatile_libtest_default<int>,
          tainted_impl<false, int, rlbox_default_tainted_testsandbox>>);

  // Check that plugin override of tainted uses the specified value
  REQUIRE(std::is_same_v<
          tainted_libtest_custom<int>,
          tainted_impl<true, int, rlbox_custom_tainted_testsandbox>>);
  REQUIRE(std::is_same_v<
          tainted_volatile_libtest_custom<int>,
          tainted_impl<false, int, rlbox_custom_tainted_testsandbox>>);
}
