#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <cassert>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << ", " << book.publication_year;
    return out;
}

}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // or
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
        std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("DeleteAuthor"s, "name"s, "Delete author"s, 
        std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
        std::bind(&View::ShowAuthorBooks, this));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty())
            throw std::logic_error("");
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            params->tags = GetBookTags();
            use_cases_.AddBook(params.value());
        }
    } catch (const std::exception&) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        std::string author_name;
        std::getline(cmd_input, author_name);
        boost::algorithm::trim(author_name);
        std::optional<std::string> author_id;
        if (author_name.empty()) {
            author_id = SelectAuthor();
        } else {
            author_id = use_cases_.GetAuthorId(author_name);
        }
        use_cases_.DeleteAuthor(*author_id);
        return true;

    } catch (const std::exception& ex) {
        //std::cout << ex.what() << std::endl;
    }
    output_ << "Failed to delete author"sv << std::endl;
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetBooks());
    return true;
}

bool View::ShowAuthorBooks() const {
    try {
        if (auto author_id = SelectAuthor()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;
    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);
    if (params.title.empty())
        return std::nullopt;
    auto author_id = EnterAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
    }
    return params;
}

std::optional<std::string> View::SelectAuthor() const {
    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;
    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }
    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }
    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }
    return authors[author_idx].id;
}

std::optional<std::string> View::EnterAuthor() const {
    output_ << "Enter author name or empty line to select from list:" << std::endl;
    std::string name;
    if (!std::getline(input_, name) || name.empty()) {
        return SelectAuthor();
    }
    try{
        return use_cases_.GetAuthorId(name);
    } catch (const std::exception&) {
    }
    output_ << "No author found. Do you want to add " << name << " (y/n)?" << std::endl;
    std::string confirm;
    if (!std::getline(input_, confirm) || confirm.empty()) {
        return std::nullopt;
    }
    if (!(confirm == "Y" || confirm == "y"))
        return std::nullopt;
    try{
        use_cases_.AddAuthor(std::move(name));
        return use_cases_.GetAuthorId(name);
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return std::nullopt;
}

std::set<std::string> View::GetBookTags() const {
    auto del_space = [&](std::string& str) {
        std::vector<std::string> splits;
        boost::split(splits, str, boost::is_any_of(" "));
        str.erase();
        for (auto& split : splits) {
            boost::algorithm::trim(split);
            if (split.empty())
                continue;
            str += split + " ";
        }
        boost::algorithm::trim(str);
        return str;
    };
    output_ << "Enter tags (comma separated):" << std::endl;
    std::string tags_str;
    if (!std::getline(input_, tags_str) || tags_str.empty()) {
        return {};
    }
    std::vector<std::string> split_tags;
    std::set<std::string> tags;
    boost::split(split_tags, tags_str, boost::is_any_of(","));
    for (auto& split_tag : split_tags) {
        del_space(split_tag);
        if (split_tag.empty())
            continue;
        tags.insert(split_tag);
    }
    std::cout << std::endl;
    return tags;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
    detail::AuthorsInfo authors_info;
    use_cases_.GetAuthors(authors_info);
    dst_autors = std::move(authors_info);
    return dst_autors;
}

std::vector<detail::BookInfo> View::GetBooks() const {
    std::vector<detail::BookInfo> books;
    detail::BooksInfo books_info = use_cases_.GetBooks();
    books = std::move(books_info);
    return books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    std::vector<detail::BookInfo> books;
    detail::BooksInfo books_info = use_cases_.GetAuthorBooks(author_id);
    books = std::move(books_info);
    return books;
}

}  // namespace ui
