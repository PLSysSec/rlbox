#include <cstdint>
#include <limits>
#include <type_traits>

#include "catch2/catch.hpp"

#define RLBOX_NO_COMPILE_CHECKS
#define RLBOX_USE_EXCEPTIONS
#include "rlbox.hpp"

class EmptySandboxType
{
public:
  using T_LongLongType = int64_t;
  using T_LongType = int32_t;
  using T_IntType = int32_t;
  using T_PointerType = uint32_t;
};

using T_Sbx = RLBoxSandbox<EmptySandboxType>;

#define UNUSED(varName) (void) varName