#pragma once
#include <pqxx/pqxx>

namespace db_books {
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;
struct Tags
{
	static inline constexpr auto ins_book = "ins_movie"_zv;
};
struct Book {
    int id = -1;
    std::string title;
    std::string author;
    int year;
    std::optional<std::string> isbn;
};
using Books = std::vector<Book>;

class BooksDB
{
public:
	BooksDB(const char* db_string);
	void CreateTable();
	bool AddBook(const Book& book);
    Books AllBooks();

private:
	pqxx::connection conn;
};


}
