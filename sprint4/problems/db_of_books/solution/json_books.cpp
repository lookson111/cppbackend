#include "json_books.h"
#include <iostream>

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
	if (action == "add_book") {
	    auto payload = jv.at("payload");
        db_books::Book book = ParseBook(payload);
		bool res = books_db_.AddBook(book);
		return Result(res);
	}
	else if (action == "all_books") {
		db_books::Books books = books_db_.AllBooks();
		return BooksConvert(books);
	}
	else if (action == "exit") {
		return Exit();
	}
    return Exit();
}

std::string JsonBooks::BooksConvert(const db_books::Books& books){
    js::array js_books;
    for (const auto &book : books) {
        js::object js_book;
        js_book["id"] = book.id;
        js_book["title"] = book.title;
        js_book["author"] = book.author;
        js_book["year"] = book.year;
        if (book.isbn.has_value()) 
            js_book["ISBN"] = *book.isbn;
        else
            js_book["ISBN"] = nullptr;
        js_books.emplace_back(js_book); 
    }
    return serialize(js_books);
}

std::string JsonBooks::Result(bool res)
{
	js::object jo;
	jo["result"] = res;
	return serialize(jo);
}

std::string JsonBooks::Exit()
{
    return "";
}

db_books::Book JsonBooks::ParseBook(js::value const js_book) {
    db_books::Book book;
    book.title = js_book.at("title").as_string();
    book.author = js_book.at("author").as_string();
    book.year = js_book.at("year").as_int64();
    auto isbn = js_book.at("ISBN");
    book.isbn = isbn.is_null() ? std::nullopt : 
        std::optional<std::string>(isbn.as_string());
    return book;
}

} // namespace json_books
