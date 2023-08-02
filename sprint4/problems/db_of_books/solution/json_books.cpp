#include "json_books.h"

#include <boost/json.hpp>

namespace json_books {
namespace js = boost::json;
JsonBooks::JsonBooks(db_books::BooksDB& books_db)
	: books_db_(books_db)
{
}
std::string JsonBooks::Command(std::string& command)
{
	js::error_code ec;
	js::value const jv = js::parse(command, ec);
	auto action = jv.at("action").as_string();
	auto payload = jv.at("payload");
	if (action == "add_book") {
		bool res = books_db_.AddBook();
		return Result(res);
	}
	else if (action == "all_books") {
		std::string all_books = books_db_.AllBooks();
		return all_books;
	}
	else if (action == "exit") {
		//bool res = books_db_.Exit();
		return "";
	}
}
void JsonBooks::Exit()
{

}
std::string JsonBooks::Result(bool res)
{
	js::object jo;
	jo["result"] = res;
	return serialize(jo);
}
} // namespace json_books