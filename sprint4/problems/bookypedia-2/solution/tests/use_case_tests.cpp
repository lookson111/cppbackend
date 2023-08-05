#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/book.h"
#include "../src/domain/book_tags.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }
    void GetAuthors(domain::Authors& authors) override {
        saved_authors.clear();
        for (auto author : authors)
            saved_authors.emplace_back(author);
    }
    domain::AuthorId GetAuthorId(const std::string& name) override {
        domain::AuthorId id = domain::AuthorId::FromString(name);
        for ( auto& author : saved_authors) {
            if (author.GetId() == id) 
                return author.GetId();
        }
        return {};
    }
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_books;

    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }
    domain::Books GetAuthorBooks(const domain::AuthorId& author_id) override {
        return saved_books;
    }
    domain::Books GetBooks() override {
        return saved_books;
    }

};

struct MockBooksTagsRepository : domain::BooksTagsRepository {
    std::vector<domain::BookTags> saved_books_tags;

    void Save(const domain::BookTags& book) override {
        saved_books_tags.emplace_back(book);
    }
    /*domain::Books GetAuthorBooks(const domain::AuthorId& author_id) override {
        return saved_books_tags;
    }
    domain::Books GetBooks() override {
        return saved_books_tags;
    }*/
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
    MockBooksTagsRepository books_tags;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases{authors, books, books_tags};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }
        }
    }
}