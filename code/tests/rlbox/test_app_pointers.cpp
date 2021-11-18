#include <stdlib.h>
#include <vector>

#include "test_include.hpp"

using rlbox::app_pointer;
using rlbox::tainted;

using RL = rlbox::rlbox_sandbox<TestSandbox>;

// NOLINTNEXTLINE
TEST_CASE("Test app pointers", "[app pointer]")
{
  RL sandbox;
  sandbox.create_sandbox();

  auto sandbox_mem_loc = sandbox.malloc_in_sandbox<unsigned int*>();

  unsigned int* ptr = (unsigned int*)malloc(sizeof(unsigned int));
  app_pointer<unsigned int*, TestSandbox> app_ptr =
    sandbox.get_app_pointer(ptr);
  tainted<unsigned int*, TestSandbox> app_ptr_tainted = app_ptr.to_tainted();

  REQUIRE(!app_ptr.is_unregistered());

  // Force the conversion to volatile and back
  *sandbox_mem_loc = app_ptr_tainted;
  tainted<unsigned int*, TestSandbox> app_ptr_tainted_reread = *sandbox_mem_loc;
  unsigned int* original_ptr = sandbox.lookup_app_ptr(app_ptr_tainted_reread);

  REQUIRE(ptr == original_ptr);

  free(ptr);

  app_ptr.unregister();
  REQUIRE(app_ptr.is_unregistered());

  sandbox.destroy_sandbox();
}
