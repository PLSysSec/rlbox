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

TEST_CASE("Type Convert function operates correctly", "[convert]" ) {
    const int32_t randValue = 5;
    auto a = rlbox::convert_fundamental<int64_t, int32_t>(randValue);
    REQUIRE(std::is_same_v<decltype(a), int64_t>);
    REQUIRE(a == randValue);
}

TEST_CASE("Type Convert function compile time checks operate correctly", "[convert]" ) {
    //Have configured rlbox to convert compile time checks to runtime throws
    const int32_t randValue = 5;
    REQUIRE_THROWS(rlbox::convert_fundamental<uint64_t, int32_t>(randValue));
    REQUIRE_THROWS(rlbox::convert_fundamental<int64_t, uint32_t>(randValue));
    // Foo a;
    // REQUIRE_THROWS(rlbox::convert_fundamental<Foo, Foo>(a));
    // REQUIRE_THROWS(rlbox::convert_fundamental<Foo, int>(randValue));
    // REQUIRE_THROWS(rlbox::convert_fundamental<int, Foo>(a));
}

TEST_CASE("Type Convert function dynamic bounds checks operate correctly", "[convert]" ) {
    uint64_t u32Max = std::numeric_limits<uint32_t>::max();
    REQUIRE(rlbox::convert_fundamental<uint32_t, uint64_t>(5) == 5);
    REQUIRE_THROWS(rlbox::convert_fundamental<uint32_t, uint64_t>(u32Max + 1));
}


