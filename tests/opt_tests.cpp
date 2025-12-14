#define CATCH_CONFIG_DEFAULT_REPORTER "console"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("opt placeholder", "[opt]") {
    REQUIRE(true);
}