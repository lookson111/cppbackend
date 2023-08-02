#include "db_books.h"

namespace db_books {

BooksDB::BooksDB(const char* db_string)
	: conn(db_string)
{
}

void BooksDB::CreateTable()
{
    auto create_table =
        "CREATE TABLE books (\""
        "id              SERIAL PRIMARY KEY NOT NULL,"
        "firm            varchar(100) NOT NULL,"
        "author          varchar(100) NOT NULL,"
        "year            integer NOT NULL,"
        "serial          char(13) UNIQUE"
        "\"); "_zv;
    pqxx::work w(conn);
    w.exec(create_table);
    conn.prepare(Tags::ins_book, "INSERT INTO books values (DEFAULT, $1, $2, $3, $4);"_zv);
}

bool BooksDB::AddBook(const std::string& title, const std::string& author, int year, std::optional<std::string> isbn)
{
    pqxx::work w(conn);
    auto res = w.exec_prepared(Tags::ins_book, title, author, year, isbn);
    w.commit();
    return true;
}



} // namespace db_books