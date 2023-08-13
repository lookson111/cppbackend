#pragma once
#include <string>
#include <vector>
#include <optional>
#include <set>
#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;
class Book {
public:
    Book(BookId book_id, Author author, std::string title, 
            std::optional<int> year,
            std::set<std::string> book_tags)
        : book_id_(std::move(book_id))
        , author_(std::move(author))
        , title_(std::move(title))
        , year_(year)
        , book_tags_(std::move(book_tags)) {
    }

    const BookId& GetBookId() const noexcept {
        return book_id_;
    }
    const Author& GetAuthor() const noexcept {
        return author_;
    }
    const std::string& GetTitle() const noexcept {
        return title_;
    }
    std::optional<int> GetYear() const noexcept {
        return year_;
    }
    std::set<std::string> GetTags() const noexcept {
        return book_tags_;
    }
    void AddTag(const std::string& tag) noexcept {
        book_tags_.insert(tag);
    }

private:
    BookId book_id_;
    Author author_;
    std::string title_;
    std::optional<int> year_;
    std::set<std::string> book_tags_;
};

using Books = std::vector<Book>;

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual void Edit(const Book& book) = 0;
    virtual Books GetAuthorBooks(const AuthorId& author_id) = 0;
    virtual Books GetBooks() = 0;
    virtual domain::Books GetBooks(const std::string& start_with) = 0;
    virtual domain::Book GetBook(const domain::BookId& book_id) = 0;
    virtual void DeleteBook(const domain::BookId& book_id) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
