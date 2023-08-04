#pragma once
#include <string>
#include <vector>
#include <optional>
#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId book_id, AuthorId author_id, std::string title, std::optional<int> year)
        : book_id_(std::move(book_id))
        , author_id_(std::move(author_id))
        , title_(std::move(title))
        , year_(year) {
    }

    const BookId& GetBookId() const noexcept {
        return book_id_;
    }
    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }
    const std::string& GetTitle() const noexcept {
        return title_;
    }
    std::optional<int> GetYear() const noexcept {
        return year_;
    }

private:
    BookId book_id_;
    AuthorId author_id_;
    std::string title_;
    std::optional<int> year_;
};

using Books = std::vector<Book>;

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual Books GetAuthorBooks(const AuthorId& author_id) = 0;
    virtual Books GetBooks() = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
