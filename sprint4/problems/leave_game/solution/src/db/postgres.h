#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/book.h"
#include "../domain/book_tags.h"

namespace postgres {

class DogRepositoryImpl : public domain::DogRepository {
public:
    explicit DogRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Dog& dog) override;
    void GetDogs(domain::Dogs &autors) override;
    domain::DogId GetDogId(const std::string& name) override;
    void DeleteDog(const domain::DogId& dog_id) override;
    void EditDog(const domain::Dog& dog) override;

private:
    pqxx::connection& connection_;
};


class Database {
public:
    explicit Database(pqxx::connection connection);

    DogRepositoryImpl& GetDogs() & {
        return dogs_;
    }

private:
    pqxx::connection connection_;
    DogRepositoryImpl dogs_{connection_};
};

}  // namespace postgres
