#pragma once

#include <string>
#include "../ui/detail.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void GetAuthors(ui::detail::AuthorsInfo& authors_info) = 0;
    virtual void AddBook(ui::detail::AddBookParams& books_info) = 0;
    virtual ui::detail::BooksInfo GetAuthorBooks(const std::string& author_id) = 0;
    virtual ui::detail::BooksInfo GetBooks() = 0;

protected:
    virtual ~UseCases() = default;
};

}  // namespace app
