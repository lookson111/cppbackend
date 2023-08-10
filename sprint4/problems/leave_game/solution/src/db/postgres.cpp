#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>
#include <iostream>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void RetiredPlayerRepositoryImpl::Save(const app::RetiredPlayer& retired_player) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4)
ON CONFLICT (id);
)"_zv,
        retired_player.GetId().ToString(), 
        retired_player.GetName(),
        retired_player.GetScore(),
        retired_player.GetPlayTime().count()
    );
    work.commit();
}

void RetiredPlayerRepositoryImpl::GetRetiredPlayers(app::RetiredPlayers& retired_players) {
    pqxx::read_transaction r{connection_};
    auto query_text = R"(SELECT id, name FROM dogs 
ORDER BY name ASC;
)"_zv;
    auto res =  r.query<std::string, std::string, unsigned, unsigned>(query_text);
    for (const auto [id, name, score, play_time] : res) {
        app::RetiredPlayer retired_player(app::PlayerId::FromString(id), name, score, app::milliseconds{play_time});
        retired_players.push_back(std::move(retired_player));
    }
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS retired_players (
    id UUID CONSTRAINT dog_id_constraint PRIMARY KEY,
    name varchar(100) NOT NULL
    score integer NOT NULL
    play_time_ms integer NOT NULL
);
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres
