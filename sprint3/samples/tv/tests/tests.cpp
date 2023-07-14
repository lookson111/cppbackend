#include <catch2/catch_test_macros.hpp>

#include "../src/tv.h"

using namespace std::literals;

namespace {
const std::string TAG = "[TV]";
}

namespace Catch {

template <>
struct StringMaker<std::nullopt_t> {
    static std::string convert(std::nullopt_t) {
        using namespace std::literals;
        return "nullopt"s;
    }
};

template <typename T>
struct StringMaker<std::optional<T>> {
    static std::string convert(const std::optional<T>& opt_value) {
        if (opt_value) {
            return StringMaker<T>::convert(*opt_value);
        } else {
            return StringMaker<std::nullopt_t>::convert(std::nullopt);
        }
    }
};

}  // namespace Catch 

SCENARIO("TV behaviour", TAG) {
    GIVEN("A TV") {
        TV tv;

        SECTION("Initially it is off and doesn't show any channel") {
            CHECK(!tv.IsTurnedOn());
            CHECK(!tv.GetChannel().has_value());
        }

        WHEN("it is turned on first time") {
            tv.TurnOn();

            THEN("it is turned on and shows channel #1") {
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 1);

                AND_WHEN("it is turned off") {
                    tv.TurnOff();

                    THEN("it is turned off and doesn't show any channel") {
                        CHECK(!tv.IsTurnedOn());
                        CHECK(tv.GetChannel() == std::nullopt);
                    }
                }
            }
        }
    }
}

/*
TEST_CASE("TV", TAG) {
    TV tv;
    SECTION("TV is off by default") {
        // Внутри внутренней секции доступны переменные, объявленные во внешних
        CHECK(!tv.IsTurnedOn());
    }
    SECTION("TV doesn't show any channel when it is off") {
        CHECK(!tv.GetChannel().has_value());
    }
    SECTION("When TV is turned on") {
        tv.TurnOn();
        // Вложенные секции проверяют состояние включенного телевизора
        SECTION("it shows channel #1 after first turning on") {
            CHECK(tv.IsTurnedOn());
            CHECK(tv.GetChannel() == 1);
        }
        SECTION("it can be turned off, so it doesn't show any channel") {
            tv.TurnOff();
            CHECK(!tv.IsTurnedOn());
            CHECK(tv.GetChannel() == std::nullopt);
        }
    }
} 
*/
/*
struct DefaultTV {
    TV tv;
};
TEST_CASE_METHOD(DefaultTV, "TV is off by default", TAG) {
    CHECK(!tv.IsTurnedOn());
}
TEST_CASE_METHOD(DefaultTV, "TV doesn't show any channel when it is off", TAG) {
    CHECK(!tv.GetChannel().has_value());
}

struct TurnedOnTV : DefaultTV {
    TurnedOnTV() {
        tv.TurnOn();
    }
};
TEST_CASE_METHOD(TurnedOnTV, "TV shows channel #1 after first turning on", TAG) {
    CHECK(tv.IsTurnedOn());
    CHECK(tv.GetChannel() == 1);
}
TEST_CASE_METHOD(TurnedOnTV, "TV turns off and doesn't show any channel", TAG) {
    tv.TurnOff();
    CHECK(!tv.IsTurnedOn());
    CHECK(tv.GetChannel() == std::nullopt);
} */
/*
TEST_CASE("TV is off by default", TAG) {
    TV tv;
    CHECK(!tv.IsTurnedOn()); // либо CHECK_FALSE(tv.IsTurnedOn());
} 

TEST_CASE("TV doesn't show any channel when it is off", TAG) {
    TV tv;
    CHECK(!tv.GetChannel().has_value());
}
TEST_CASE("TV shows channel #1 after first turning on", TAG) {
    TV tv;
	tv.TurnOn();
    CHECK(tv.IsTurnedOn());
    CHECK(tv.GetChannel() == 1);
}
TEST_CASE("TV after turning off turns off and doesnt show any channel", TAG) {
    TV tv;
	tv.TurnOn();
	tv.TurnOff();
    CHECK(!tv.IsTurnedOn());
    CHECK(!tv.GetChannel().has_value());
}*/
// Напишите недостающие тесты самостоятельно
