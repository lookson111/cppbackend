#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "../src/model/model.h"
#include "../src/infrastructure/model_serialization.h"
#include "../src/json_loader.h"

using namespace model;
using namespace std::literals;
namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

}  // namespace

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const geom::Point2D p{10, 20};
        WHEN("point is serialized") {
            output_archive << p;

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                geom::Point2D restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            //Dog dogz{Dog::Id{42}, "Pluto"s, {42.2, 12.5}, 3};
            Dog dog{Dog::Id{42}, "Pluto"s, geom::Point2D{42.2, 12.5}};
            dog.AddScore(42);
            Loots loots{ { Loot::Id{10}, 2u, Point2D{0.0, 0.0} } };
            dog.PutTheLoot(loots, loots.begin());
            CHECK(dog.GetLoots().size() == 1);
            dog.SetDirection(Direction::EAST);
            dog.SetSpeed({2.3, -1.2});
            return dog;
        }();
        auto game = [] {
            model::Game game = 
                json_loader::LoadGame("../tests/config_test.json"sv);
            return game;
        } ();
        auto game_session = [&game] {
            auto map_id = game.GetMaps().front().GetId();
            auto game_session = game.AddGameSession(map_id);
            return game_session;
        }();
        auto dog_nop = [&game_session] {
            auto dog = game_session->AddDog("nop");
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(dog.GetId() == restored.GetId());
                CHECK(dog.GetName() == restored.GetName());
                CHECK(dog.GetPoint() == restored.GetPoint());
                CHECK(dog.GetSpeed() == restored.GetSpeed());
                //CHECK(dog.GetBagCapacity() == restored.GetBagCapacity());
                CHECK(dog.GetLoots() == restored.GetLoots());
            }
        }
        AND_WHEN("game session is serialized") {
            {
                serialization::GameSessionRepr repr{*game_session};
                output_archive << repr;
            }
            
            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::GameSessionRepr repr;
                input_archive >> repr;
            }
        }
        AND_WHEN("game is serialized") {
            {
                serialization::GameRepr repr{game};
                output_archive << repr;
            }
            
            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::GameRepr repr;
                input_archive >> repr;
            /*    model::Game game_r = 
                    json_loader::LoadGame("../tests/config_test.json"sv);
                repr.Restore(game_r);
                CHECK(game_r.GetGameSessions().size() == 
                    game.GetGameSessions().size());*/
            }


        }
    }
}
