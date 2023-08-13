#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/book.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Author& author) override;
    void GetAuthors(domain::Authors &autors) override;

private:
    pqxx::connection& connection_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Book& book) override;
    domain::Books GetAuthorBooks(const domain::AuthorId& author_id) override;
    domain::Books GetBooks() override;

private:
    pqxx::connection& connection_;

    domain::Books ConvertResponseToBooks(auto resp) {
        domain::Books books;
        for (const auto [id, author_id, title, year] : resp) {
            domain::Book book(domain::BookId::FromString(*id),
                domain::AuthorId::FromString(*author_id), 
                *title,
                year
            );
            books.push_back(std::move(book));
        }
        return books;
    }
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() & {
        return authors_;
    }
    BookRepositoryImpl& GetBooks() & {
        return books_;
    }

private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl books_{connection_};
};

}  // namespace postgres
