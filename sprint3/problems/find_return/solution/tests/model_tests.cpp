#include <cmath>
#include <catch2/catch_test_macros.hpp>

#include "../src/model.h"

using namespace std::literals;

SCENARIO("Game model") {
	using model::Game;

	GIVEN("a game model") {
		Game game(model::LootGeneratorConfig{});
		WHEN("maps not added") {
			CHECK(game.GetMaps().empty());
		}

	}
}
