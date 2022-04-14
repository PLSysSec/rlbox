// Check that the tainted type is correctly reflected in rlbox_sandbox when:
// - the sandbox plugin leaves the tainted type as default
// - the sandbox plugin overrides the tainted type

#include "test_include.hpp"

#include "rlbox_tainted_fixed_aligned.hpp"
#include "rlbox_tainted_relocatable.hpp"

#include <type_traits>

namespace rlbox {

class rlbox_default_tainted_testsandbox
{};

class rlbox_custom_tainted_testsandbox
{
public:
  template<typename T>
  using tainted = tainted_fixed_aligned<T, rlbox_custom_tainted_testsandbox>;
};

}

using namespace rlbox;

RLBOX_DEFINE_BASE_TYPES_FOR(libtest_default_tainted,
                            rlbox_default_tainted_testsandbox);
RLBOX_DEFINE_BASE_TYPES_FOR(libtest_custom_tainted,
                            rlbox_custom_tainted_testsandbox);

TEST_CASE("Test tainted reflection", "[rlbox plugin reflection]")
{
  // Check that by default, sandboxes assume that the plugin uses
  // tainted_relocatable If the plugin overrides this choice, use the specified
  // value instead

  REQUIRE(std::is_same_v<
          rlbox_sandbox_libtest_default_tainted::tainted<int>,
          tainted_relocatable<int, rlbox_default_tainted_testsandbox>>);

  rlbox_test_helper_print_type<
    rlbox_sandbox_libtest_custom_tainted::tainted<int>,
    tainted_fixed_aligned<int, rlbox_custom_tainted_testsandbox>>();

  // Disable this test for now as this is failing
  REQUIRE(std::is_same_v<
          rlbox_sandbox_libtest_custom_tainted::tainted<int>,
          tainted_fixed_aligned<int, rlbox_custom_tainted_testsandbox>>);
}