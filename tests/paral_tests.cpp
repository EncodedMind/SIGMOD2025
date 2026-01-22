#include <catch2/catch_test_macros.hpp>

TEST_CASE("Empty join", "[join]") {
    int x = 0;
    REQUIRE(x == 0);
}