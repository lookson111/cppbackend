#pragma once
#include "sdk.h"
#include <boost/json.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <random>
#include "model.h"
#include "token.h"

namespace app {
namespace js = boost::json;
namespace net = boost::asio;
namespace sys = boost::system;

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
};

std::string JsonMessage(std::string_view code, std::string_view message);
//std::string_view GetToken(std::string_view autorization_text);

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    // Функция handler будет вызываться внутри strand с интервалом period
    Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
        : strand_{ strand }
        , period_{ period }
        , handler_{ std::move(handler) } {
    }

    void Start() {
        if (period_ == std::chrono::milliseconds(0))
            return;
        net::dispatch(strand_, [self = shared_from_this()] {
            self->last_tick_ = Clock::now();
            self->ScheduleTick();
            });
    }

private:
    void ScheduleTick() {
        assert(strand_.running_in_this_thread());
        timer_.expires_after(period_);
        timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
            self->OnTick(ec);
            });
    }

    void OnTick(sys::error_code ec) {
        using namespace std::chrono;
        assert(strand_.running_in_this_thread());

        if (!ec) {
            auto this_tick = Clock::now();
            auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
            last_tick_ = this_tick;
            try {
                handler_(delta);
            }
            catch (...) {
            }
            ScheduleTick();
        }
    }

    using Clock = std::chrono::steady_clock;

    Strand strand_;
    std::chrono::milliseconds period_;
    net::steady_timer timer_{strand_};
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};

class Player {
public:
    using Id = util::Tagged<uint64_t, Player>;
    Player(model::GameSession* session, model::Dog* dog) : 
        session_(session), dog_(dog), id_({idn++}) {}
    const Id& GetId() const {
        return id_;
    }
    const model::Map::Id& MapId() const {
        return session_->MapId();
    }
    std::string_view GetName() const {
        return dog_->GetName();
    }
    const model::GameSession* GetSession() {
        return session_;
    }
    void Move(std::string_view move_cmd);

private:
    static std::atomic<uint64_t> idn;
    Id id_;
    model::GameSession* session_;
    model::Dog* dog_;
};

//namespace detail {
//    struct TokenTag {};
//}  // namespace detail

//using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    Player* FindPlayer(Token token) const;
    Token AddPlayer(Player* player);

private:
    std::unordered_map<Token, Player*, util::TaggedHasher<Token>> token_to_player;
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
    Player* Add(model::Dog *dog, model::GameSession *session);
    Player* FindPlayer(Player::Id player_id, model::Map::Id map_id) noexcept;
    const Player::Id* FindPlayerId(std::string_view player_name) const noexcept;
    Player* FindPlayer(Player::Id player_id) const noexcept;
    
private:
    using PlayerIdHasher = util::TaggedHasher<Player::Id>;
    using PlayerIdToIndex = std::unordered_map<Player::Id, size_t, PlayerIdHasher>;
    std::deque<std::unique_ptr<Player>> players_;
    PlayerIdToIndex player_id_to_index_;
};

class App
{
public:
    explicit App(model::Game& game)
        : game_{ game } {
    }
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
    Player* GetPlayer(const Token& token) const;
    Player* GetPlayer(std::string_view nick, std::string_view mapId);
};
}

