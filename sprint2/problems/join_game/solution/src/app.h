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



class Player {
public:
private:
    GameSession* session_;
    Dog* dog_;
};

namespace detail {
    struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    Player* FindPlayer(Token token);
    Token AddPlayer(Player* player);

private:
    std::unordered_map<Token, Player*> token_to_player;
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::string GetToken() {
        std::string r1 = ToHex(generator1_());
        std::string r2 = ToHex(generator2_());
        return r1 + r2;
    }
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным
    std::string ToHex(uint64_t n) const;
};


class Players {
public:
    Player Add(Dog dog, GameSession session);
    Player* FindPlayer(DogId id, MapId id);
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

