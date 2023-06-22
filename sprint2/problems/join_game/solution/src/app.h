#pragma once
#include "sdk.h"
#include <boost/json.hpp>
#include <string>
#include <random>
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

namespace detail {
    struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
    //...
public:
    std::string GetToken() {
        std::string r1 = ToHex(generator1_());
        std::string r2 = ToHex(generator2_());
        return r1 + r2;
    }
private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    // ����� ������������� �����, �������� �� generator1_ � generator2_
    // ��� 64-��������� ����� �, �������� �� � hex-������, ������� � ����.
    // �� ������ �������������������� � ���������� ������������� �������,
    // ����� ������� �� ������ ��� ����� ���������������
    std::string ToHex(uint64_t n) const;
};


class Players {

};

class App
{
public:
    explicit App(model::Game& game)
        : game_{ game } {
    }
    std::pair<std::string, bool> GetMapBodyJson(std::string_view requestTarget) const;
    std::pair<std::string, JoinError> ResponseJoin(std::string_view jsonBody);
    std::pair<std::string, error_code> GetPlayers(std::string_view token) const;
    
private:
    model::Game& game_;
};
}

