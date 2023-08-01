#pragma once
#include "sdk.h"
#include <boost/json.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <random>
#include "model/model.h"
#include "token.h"
#include "ticker.h"

namespace app {
namespace js = boost::json;
namespace net = boost::asio;
namespace sys = boost::system;
using milliseconds = std::chrono::milliseconds;
using Token = security::Token;

enum class JoinError {
    None,
    BadJson,
    MapNotFound,
    InvalidName
};
enum class error_code {
    None,
    InvalidToken,
    UnknownToken,
    InvalidArgument
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
    static js::value ToJsonValue(std::string_view json_body);
};

std::string JsonMessage(std::string_view code, std::string_view message);

class Player {
public:
    using Id = util::Tagged<uint64_t, Player>;
    Player() = default;
    Player(Id id, model::GameSession* session, model::Dog* dog) : 
        session_(session), dog_(dog), id_(id) {}
    const Id& GetId() const {
        return id_;
    }
    const model::Map::Id& MapId() const {
        return session_->MapId();
    }
    std::string_view GetName() const {
        return dog_->GetName();
    }
    const model::GameSession* GetSession() const {
        return session_;
    }
    const model::Dog* GetDog() const {
        return dog_;
    }
    void Move(std::string_view move_cmd);
    Player(const Player& other) {
        id_ = other.id_;
        session_ = other.session_;
        dog_ = other.dog_;
    }
    Player(Player&& other) {
        id_ = std::move(id_);
        session_ = other.session_;
        dog_ = other.dog_;
    }
private:
    Id id_{0};
    model::GameSession* session_ = nullptr;
    model::Dog* dog_ = nullptr;
};

class PlayerTokens {
public:
    using TokenToPlayerContainer = std::unordered_map<Token, Player*,
        util::TaggedHasher<Token>>;
    Player* FindPlayer(Token token) const;
    Token AddPlayer(Player* player);
    void AddToken(Token token, Player* player);
    const TokenToPlayerContainer& GetTokens() const;

private:
    TokenToPlayerContainer token_to_player;
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::string GetToken();
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным
    std::string ToHex(uint64_t n) const;
};


class Players {
public:
    using PlayerContainer = std::unique_ptr<Player>;
    using PlayersContainer = std::deque<PlayerContainer>;
    Player* Add(Player::Id player_id, model::Dog *dog, model::GameSession *session);
    void Add(PlayerContainer&& player);
    Player* FindPlayer(Player::Id player_id, model::Map::Id map_id) noexcept;
    const Player::Id* FindPlayerId(std::string_view player_name) const noexcept;
    Player* FindPlayer(Player::Id player_id) const noexcept;
    const PlayersContainer& GetPlayers() const;
    
private:
    using PlayerIdHasher = util::TaggedHasher<Player::Id>;
    using PlayerIdToIndex = std::unordered_map<Player::Id, size_t, PlayerIdHasher>;
    PlayersContainer players_;
    PlayerIdToIndex player_id_to_index_;
    Player* PushPlayer(PlayerContainer&& player);
};

class App
{
public:
    explicit App(model::Game& game)
        : game_{ game } {
    }
    model::Game& GetGameModel();
    void SetLastPlayerId(Player::Id id);
    Player::Id GetLastPlayerId() const;
    const Players& GetPlayers() const;
    const PlayerTokens& GetPlayerTokens() const;
    Players& EditPlayers();
    PlayerTokens& EditPlayerTokens();

    std::pair<std::string, bool> GetMapBodyJson(std::string_view requestTarget) const;
    std::pair<std::string, JoinError> ResponseJoin(std::string_view jsonBody);
    std::pair<std::string, error_code> ActionMove(
        const Token& token, std::string_view jsonBody);
    std::pair<std::string, error_code> Tick(std::string_view jsonBody);
    std::pair<std::string, error_code> GetPlayers(const Token& token) const;
    std::pair<std::string, error_code> GetState(const Token& token) const;
    std::pair<std::string, error_code> CheckToken(const Token& token) const;
    
private:
    model::Game& game_;
    Players players_;
    PlayerTokens player_tokens_;
    Player::Id last_player_id_{0};
    Player* GetPlayer(const Token& token) const;
    Player* GetPlayer(std::string_view nick, std::string_view mapId);
    auto GetJsonDogBag(const model::Dog& dog) const;
};
}

