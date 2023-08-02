#pragma once
#include <pqxx/pqxx>

namespace db_books {
struct Tags
{
	static inline constexpr auto ins_book = "ins_movie"_zv;
};

// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;
class BooksDB
{

public:
	BooksDB(const char* db_string);
	void CreateTable();
	bool AddBook(const std::string& title, const std::string& author, int year, std::optional<std::string> isbn);
private:
	pqxx::connection conn;
};


}
