#include <limits>
#include <type_traits>

#include "catch2/catch.hpp"

#define RLBOX_NO_COMPILE_CHECKS
#define RLBOX_USE_EXCEPTIONS
#include "rlbox.hpp"
#undef RLBOX_NO_COMPILE_CHECKS
#undef RLBOX_USE_EXCEPTIONS

struct Foo {
    int a;
};

using namespace rlbox;
using namespace std;

TEST_CASE("Type Convert function operates correctly", "[convert]" ) {
    auto a = convert_fundamental<long, int>(5);
    REQUIRE(is_same_v<decltype(a), long>);
    REQUIRE(a == 5);
}

TEST_CASE("Type Convert function compile time checks operate correctly", "[convert]" ) {
    //Have configured rlbox to convert compile time checks to runtime throws
    REQUIRE_THROWS(convert_fundamental<unsigned long, int>(5));
    REQUIRE_THROWS(convert_fundamental<long, unsigned int>(5));
    Foo a;
    REQUIRE_THROWS(convert_fundamental<Foo, Foo>(a));
    REQUIRE_THROWS(convert_fundamental<Foo, int>(5));
    REQUIRE_THROWS(convert_fundamental<int, Foo>(a));
}

TEST_CASE("Type Convert function dynamic bounds checks operate correctly", "[convert]" ) {
    uint64_t u32Max = numeric_limits<uint32_t>::max();
    REQUIRE(convert_fundamental<uint32_t, uint64_t>(5) == 5);
    REQUIRE_THROWS(convert_fundamental<uint32_t, uint64_t>(u32Max + 1));
}


