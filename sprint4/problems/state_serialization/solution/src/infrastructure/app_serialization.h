#pragma once

#include "../app.h"
#include "model_serialization.h"

namespace serialization {


class PlayerRepr {
public:
    PlayerRepr() = default;

    explicit PlayersRepr(const app::Player& player)
        : player_id_(player.GetId())
        , game_session_id_(player.GetGameSessionId())
        , dog_id(player.GetDogId())
    {
        
    }

    [[nodiscard]] std::unique_ptr<app::Player> Restore(const model::Game& game) const {
        Dog* dog = game.GetDog(dog_id);
        GameSession* session = game.GetGameSession(game_session_id_);
        return std::make_unique<app::Player>(id, dog, session);    
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& player_id_;
        ar& game_session_id_;
        ar& dog_id_;
    }

private:
   app::Player::Id player_id_;
   model::GameSession::Id game_session_id_;
   model::Dog::Id dog_id_;
};

class PlayersRepr {
public:
    PlayersRepr() = default;

    explicit PlayersRepr(const app::Players& players)        
    {
        for(auto &player : players.GetPlayers()) 
            players_repr_.push_back(PlayerRepr(player));
    }

    [[nodiscard]] app::Players Restore(const model::Game& game) const {
        app::Players players;
        for (auto &player_repr : players_repr) {

            players.Add();
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& players_repr_;
    }

private:
    std::list<PlayerRepr> players_repr_;
};

class AppRepr {
public:
    AppRepr() = default;

    explicit AppRepr(const app::App& app)
        : game_repr_(app.GetGameModel())
        , players_repr_(app.GetPlayers())
        , player_tokens_repr_(app.GetPlayerTokens()) 
    {
    }

    void Restore(app::App& app) const {
        game_repr_.Restore(app.GetGameModel());
        
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& game_repr_;
        ar& players_repr_;
        ar& player_tokens_repr_;
    }

private:
    GameRepr game_repr_;
    PlayersRepr players_repr_;
    PlayerTokensRepr player_token_repr_;
};

}  // namespace serialization
