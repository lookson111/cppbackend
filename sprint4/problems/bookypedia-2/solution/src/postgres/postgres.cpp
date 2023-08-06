#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>
#include <iostream>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

void AuthorRepositoryImpl::GetAuthors(domain::Authors &autors) {
    pqxx::read_transaction r{connection_};
    auto query_text = R"(SELECT id, name FROM authors 
ORDER BY name ASC;
)"_zv;
    auto res =  r.query<std::string, std::string>(query_text);
    for (const auto [id, name] : res) {
        domain::Author author(domain::AuthorId::FromString(id), name);
        autors.push_back(std::move(author));
    }
}

domain::AuthorId AuthorRepositoryImpl::GetAuthorId(const std::string& name){
    pqxx::read_transaction r{connection_};
    auto query_text = "SELECT id FROM authors "
        "WHERE name=" + r.quote(name) + 
        "LIMIT 1;";
    auto [author_id] =  r.query1<std::string>(query_text);
    return domain::AuthorId::FromString(author_id);
}

void AuthorRepositoryImpl::DeleteAuthor(const domain::AuthorId& author_id) {
    pqxx::work work{connection_};
    // get book list
    std::string author = author_id.ToString();
    auto query_text = "SELECT id FROM books "
        "WHERE author_id=" + work.quote(author) + ";";
    auto books =  work.query<std::string>(query_text);
    // delete tags and books
    for (auto &[book] : books) { 
        work.exec_params(
            R"(DELETE FROM book_tags WHERE book_id=$1;)"_zv,
            book
        );
    }
    work.exec_params(
        R"(DELETE FROM books WHERE author_id=$1;)"_zv,
        author
    );
    // delete authors
    work.exec_params(
        R"(DELETE FROM authors WHERE id=$1;)"_zv,
        author
    );
    work.commit();
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4);
)"_zv,
        book.GetBookId().ToString(), 
        book.GetAuthorId().ToString(), 
        book.GetTitle(),
        book.GetYear()
    );
    for (const auto& tag : book.GetTags()) {
        work.exec_params(
            R"(
INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);
)"_zv,
            book.GetBookId().ToString(), 
            tag
        );
    }
    work.commit();
}

domain::Books BookRepositoryImpl::GetAuthorBooks(const domain::AuthorId& author_id) {
    using o_str = std::optional<std::string>;
    using o_int = std::optional<int>;
    pqxx::read_transaction r{connection_};
    domain::Books books;
    auto query_text = "SELECT id, author_id, title, publication_year FROM books "
        "WHERE author_id=" + r.quote(author_id.ToString()) + 
        "ORDER BY publication_year ASC, title ASC;";
    return ConvertResponseToBooks(r.query<o_str, o_str, o_str, o_int>(query_text));
}

domain::Books BookRepositoryImpl::GetBooks() {
    using o_str = std::optional<std::string>;
    using o_int = std::optional<int>;
    pqxx::read_transaction r{connection_};
    domain::Books books;
    auto query_text = "SELECT id, author_id, title, publication_year FROM books "
        "ORDER BY title ASC;";
    return ConvertResponseToBooks(r.query<o_str, o_str, o_str, o_int>(query_text));
}


void BooksTagsRepositoryImpl::Save(const domain::BookTags& book_tags) {
    pqxx::work work{connection_};
    for (const auto& tag :book_tags.GetTags()) {
        work.exec_params(
            R"(
INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);
)"_zv,
            book_tags.GetBookId().ToString(), 
            tag
        );
    }
    work.commit();
}


Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id                  UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id           UUID REFERENCES authors (id) NOT NULL, 
    title               varchar(100) NOT NULL,
    publication_year    integer
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id             UUID REFERENCES books (id) NOT NULL, 
    tag                 varchar(30) NOT NULL
);
)"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres
