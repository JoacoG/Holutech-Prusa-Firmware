#include "catch2/catch_test_macros.hpp"
#include <stdint.h>
#include "power_panic_math.h"

TEST_CASE("power_panic_host_resume_line", "[power_panic]") {
    SECTION("empty planner and command queue") {
        REQUIRE(power_panic_host_resume_line(1000, 0, 0) == 1000);
    }

    SECTION("buffered planner and queue lines are subtracted") {
        REQUIRE(power_panic_host_resume_line(1000, 3, 4) == 993);
    }

    SECTION("only planner lines buffered") {
        REQUIRE(power_panic_host_resume_line(1000, 3, 0) == 997);
    }

    SECTION("only command queue blocks buffered") {
        REQUIRE(power_panic_host_resume_line(1000, 0, 4) == 996);
    }

    SECTION("fresh line counter, nothing sent yet") {
        REQUIRE(power_panic_host_resume_line(0, 0, 0) == 0);
    }
}