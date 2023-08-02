#pragma once
#include <string>
#include "db_books.h"

namespace json_books {
class JsonBooks {
public:
	JsonBooks(db_books::BooksDB& books_db);
	std::string Command(std::string& command);
	void Exit();
private:
	db_books::BooksDB& books_db_;
	std::string Result(bool res);
};
} // namespace json_books
