#include "db_books.h"
#include <iostream>
namespace db_books {

BooksDB::BooksDB(const char* db_string)
    : conn(db_string)
{
}

void BooksDB::CreateTable()
{
    auto create_table =
        "CREATE TABLE IF NOT EXISTS books ("
        "id              SERIAL PRIMARY KEY NOT NULL,"
        "title           varchar(100) NOT NULL,"
        "author          varchar(100) NOT NULL,"
        "year            integer NOT NULL,"
        "ISBN            char(13) UNIQUE"
        ");"_zv;
    pqxx::work w(conn);
    w.exec(create_table);
    w.commit();
    conn.prepare(Tags::ins_book, "INSERT INTO books values (DEFAULT, $1, $2, $3, $4);"_zv);
}

bool BooksDB::AddBook(const Book& book)
{
    pqxx::work w(conn);
    try {
        auto res = w.exec_prepared(Tags::ins_book, 
        book.title, 
        book.author, 
        book.year, 
        book.isbn);
    } catch (const std::exception& e) {
        return false;
    }
    w.commit();
    return true;
}
Books BooksDB::AllBooks() {
    using o_int = std::optional<int>;
    using o_string = std::optional<std::string>;
    Books books;
    pqxx::read_transaction r(conn);
    auto query_text = "SELECT id, title, author, year, ISBN FROM books "
        "ORDER BY year DESC, title ASC, author ASC, ISBN ASC;"_zv;
    // Выполняем запрос и итерируемся по строкам ответа
    for (auto [id, title, author, year, isbn] : 
            r.query<o_int, o_string, o_string, o_int, o_string>(query_text)) {
        Book book;
        book.id = *id;
        book.title = *title;
        book.author = *author;
        book.year = *year;
        book.isbn = isbn;
        books.push_back(book);
    }
    return books;
}
} // namespace db_books
