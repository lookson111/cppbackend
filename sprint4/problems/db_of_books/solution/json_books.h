#pragma once
#include <string>
#include <boost/json.hpp>
#include "db_books.h"

namespace json_books {
namespace js = boost::json;
class JsonBooks {
public:
	JsonBooks(db_books::BooksDB& books_db);
	std::string Command(std::string& command);
private:
	db_books::BooksDB& books_db_;

	std::string Result(bool res);
    std::string BooksConvert(const db_books::Books& books);
    db_books::Book ParseBook(js::value const js_book);
	std::string Exit();
};
} // namespace json_books
