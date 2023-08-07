#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>
#include <iostream>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void DogRepositoryImpl::Save(const domain::Dog& dog) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO dogs (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        dog.GetId().ToString(), dog.GetName());
    work.commit();
}

void DogRepositoryImpl::EditDog(const domain::Dog& dog) {
    pqxx::work work{connection_};
    auto ret = work.exec_params(
        R"(UPDATE dogs SET name=$1 WHERE id=$2 RETURNING id;)"_zv,
        dog.GetName(),
        dog.GetId().ToString()
    );
    if (ret.size() != 1)
        throw std::logic_error("Dog deleted.");
    work.commit();
}

void DogRepositoryImpl::GetDogs(domain::Dogs &autors) {
    pqxx::read_transaction r{connection_};
    auto query_text = R"(SELECT id, name FROM dogs 
ORDER BY name ASC;
)"_zv;
    auto res =  r.query<std::string, std::string>(query_text);
    for (const auto [id, name] : res) {
        domain::Dog dog(domain::DogId::FromString(id), name);
        autors.push_back(std::move(dog));
    }
}

domain::DogId DogRepositoryImpl::GetDogId(const std::string& name){
    pqxx::read_transaction r{connection_};
    auto query_text = "SELECT id FROM dogs "
        "WHERE name=" + r.quote(name) + 
        "LIMIT 1;";
    auto [dog_id] =  r.query1<std::string>(query_text);
    return domain::DogId::FromString(dog_id);
}

void DogRepositoryImpl::DeleteDog(const domain::DogId& dog_id) {
    pqxx::work work{connection_};
    // get book list
    std::string dog = dog_id.ToString();
    auto query_text = "SELECT id FROM books "
        "WHERE dog_id=" + work.quote(dog) + ";";
    auto books =  work.query<std::string>(query_text);
    // delete tags and books
    for (auto &[book] : books) { 
        work.exec_params(
            R"(DELETE FROM book_tags WHERE book_id=$1;)"_zv,
            book
        );
    }
    work.exec_params(
        R"(DELETE FROM books WHERE dog_id=$1;)"_zv,
        dog
    );
    // delete dogs
    work.exec_params(
        R"(DELETE FROM dogs WHERE id=$1;)"_zv,
        dog
    );
    work.commit();
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS dogs (
    id UUID CONSTRAINT dog_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres
