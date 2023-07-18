#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include "../src/collision_detector.h"
using namespace collision_detector;
// Напишите здесь тесты для функции collision_detector::FindGatherEvents
namespace Catch {
    template<>
    struct StringMaker<collision_detector::GatheringEvent> {
        static std::string convert(collision_detector::GatheringEvent const& value) {
            std::ostringstream tmp;
            tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

            return tmp.str();
        }
    };
}  // namespace Catch 
static const ItemGatherer::Items one_item = { Item{.position{0.0, 5.0}, .width = 0.2 } };
static const ItemGatherer::Gatherers one_gatherer =
    { Gatherer{.start_pos{0.0, 0.0}, .end_pos{0.0, 10.0}, .width = 0.3} };


SCENARIO("Collision detector", "[Collision detector]") {
    GIVEN("Item gatherer container") {
        ItemGatherer item_gatherer;
        WHEN("Conteiner emtpy") {
            CHECK(item_gatherer.GatherersCount() == 0);
            CHECK(item_gatherer.ItemsCount() == 0);
        }
        THEN("Add one item") {
            item_gatherer.Set(one_item);
            THEN("one item") {
                CHECK(item_gatherer.ItemsCount() == 1);
            }
            AND_THEN("check item") {
                CHECK(item_gatherer.GetItem(0) == one_item[0]);
            }
        }
        AND_THEN("Add gatherer") {
            item_gatherer.Set(one_gatherer);
            THEN("one getharer") {
                CHECK(item_gatherer.GatherersCount() == 1);
            }
            AND_THEN("check gatherer") {
                CHECK(item_gatherer.GetGatherer(0) == one_gatherer[0]);
            }
        }
    }
    AND_GIVEN("Item gatherer container for check collision") {
        ItemGatherer item_gatherer;
        WHEN("One item and one gatheres") {
            item_gatherer.Set(one_item);
            item_gatherer.Set(one_gatherer);


        }
    }
}

