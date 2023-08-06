#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(
        domain::AuthorRepository& authors, 
        domain::BookRepository& book,
        domain::BooksTagsRepository& books_tags)
        : authors_{authors}
        , books_{book} 
        , books_tags_(books_tags) {
    }

    void AddAuthor(const std::string& name) override;
    void GetAuthors(ui::detail::AuthorsInfo& authors_info) override;
    std::string GetAuthorId(const std::string& name) override;
    void DeleteAuthor(const std::string& author_id) override;
    void EditAuthor(ui::detail::AuthorInfo& author_info) override;

    void AddBook(ui::detail::AddBookParams& book_params) override;
    ui::detail::BooksInfo GetAuthorBooks(const std::string& author_id) override;
    ui::detail::BooksInfo GetBooks() override;

    void AddBookTags(ui::detail::BookTagsInfo& book_tags) override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
    domain::BooksTagsRepository& books_tags_;

    ui::detail::BooksInfo ConvertBooksToBooksInfo(auto&& callback) {
        ui::detail::BooksInfo books_info;
        auto books = callback();
        for (auto book : books) {
            ui::detail::BookInfo book_info(
                book.GetTitle(),
                book.GetAuthor().GetName(),
                book.GetYear().value()
            );
            books_info.push_back(std::move(book_info));
        }
        return books_info;
    }
};

}  // namespace app
