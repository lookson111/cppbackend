#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "player_repository.h"

namespace postgres {

class RetiredPlayerRepositoryImpl : public app::RetiredPlayerRepository {
public:
    explicit RetiredPlayerRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const app::RetiredPlayer& retired_player) override;
    void GetRetiredPlayers(app::RetiredPlayers & retired_players) override;

private:
    pqxx::connection& connection_;
};


class Database {
public:
    explicit Database(pqxx::connection connection);

    RetiredPlayerRepositoryImpl& GetDogs() & {
        return retired_players_;
    }

private:
    pqxx::connection connection_;
    RetiredPlayerRepositoryImpl retired_players_{connection_};
};

}  // namespace postgres
