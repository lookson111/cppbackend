#pragma once
#include <string>
#include <set>
#include "book.h"

namespace domain {

using Tags = std::set<std::string>;

class BookTags {
public:
    BookTags(BookId book_id, Tags tags)
        : book_id_(std::move(book_id))
        , tags_(std::move(tags)) {
    }

    const BookId& GetBookId() const noexcept {
        return book_id_;
    }
    const Tags& GetTags() const noexcept {
        return tags_;
    }

private:
    BookId book_id_;
    Tags tags_;
};

using BooksTags = std::vector<BookTags>;

class BooksTagsRepository {
public:
    virtual void Save(const BookTags& book) = 0;
    //virtual Books GetAuthorBooks(const AuthorId& author_id) = 0;
    //virtual Books GetBooks() = 0;

protected:
    ~BooksTagsRepository() = default;
};

}