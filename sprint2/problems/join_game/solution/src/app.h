#pragma once
#include <boost/json.hpp>
#include <string>
#include "model.h"

namespace app {
namespace js = boost::json;

enum class JoinError {
    None,
    BadJson,
    MapNotFound,
    InvalidName
};
enum class error_code {
    None,
    InvalidToken,
    UnknownToken
};

class ModelToJson {
public:
    explicit ModelToJson(model::Game& game)
        : game_{ game } {
    }
    std::string GetMaps();
    std::string GetMap(std::string_view nameMap);
private:
    model::Game& game_;
    static js::array GetRoads(const model::Map::Roads& roads);
    static js::array GetBuildings(const model::Map::Buildings& buildings);
    static js::array GetOffice(const model::Map::Offices& offices);
};

std::string JsonMessage(std::string_view code, std::string_view message);




class Players {

};

class App
{
public:
    explicit App(model::Game& game)
        : game_{ game } {
    }
    std::pair<std::string, bool> GetMapBodyJson(std::string_view requestTarget) const;
    std::pair<std::string, JoinError> ResponseJoin(std::string_view jsonBody) const;
    std::pair<std::string, error_code> GetPlayers(std::string_view token) const;
    
private:
    model::Game& game_;
};
}

