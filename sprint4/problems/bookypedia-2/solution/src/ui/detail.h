#pragma once

#include <string>
#include <vector>
#include <set>

namespace ui {
namespace detail {
using Tags = std::set<std::string>;

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
    Tags tags;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string title;
    int publication_year;
};

struct BookTagsInfo {
    std::string book_id;
    std::set<std::string> tags;
};

using AuthorsInfo = std::vector<AuthorInfo>;
using BooksInfo = std::vector<BookInfo>;
using BooksTagsInfo = std::vector<BookTagsInfo>;

} // namespace detail
} // namespace ui
