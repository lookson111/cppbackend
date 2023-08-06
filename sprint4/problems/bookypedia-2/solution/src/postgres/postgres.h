#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/book.h"
#include "../domain/book_tags.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::Author& author) override;
    void GetAuthors(domain::Authors &autors) override;
    domain::AuthorId GetAuthorId(const std::string& name) override;
    void DeleteAuthor(const domain::AuthorId& author_id) override;
    void EditAuthor(const domain::Author& author) override;

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
    domain::Books GetBooks(const std::string& start_with) override;
    domain::Book GetBook(const domain::BookId& book_id) override;
    void DeleteBook(const domain::BookId& book_id) override;

private:
    using o_str = std::optional<std::string>;
    using o_int = std::optional<int>;
    pqxx::connection& connection_;

    domain::Books ConvertResponseToBooks(auto resp) {
        domain::Books books;
        domain::Tags tags;
        for (const auto [id, author_id, author_name, title, year] : resp) {
            domain::Book book(domain::BookId::FromString(*id),
                domain::Author{domain::AuthorId::FromString(*author_id), *author_name}, 
                *title,
                year,
                tags
            );
            books.push_back(std::move(book));
        }
        return books;
    }
};


class BooksTagsRepositoryImpl : public domain::BooksTagsRepository {
public:
    explicit BooksTagsRepositoryImpl(pqxx::connection& connection)
        : connection_{connection} {
    }

    void Save(const domain::BookTags& book) override;
    
private:
    pqxx::connection& connection_;
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
    BooksTagsRepositoryImpl& GetBooksTags() & {
        return books_tags_;
    }

private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl books_{connection_};
    BooksTagsRepositoryImpl books_tags_{connection_};
};

}  // namespace postgres
