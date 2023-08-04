#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, 
        domain::BookRepository& book)
        : authors_{authors}
        , books_{book} {
    }

    void AddAuthor(const std::string& name) override;
    void GetAuthors(ui::detail::AuthorsInfo& authors_info) override;
    void AddBook(ui::detail::AddBookParams& book_params) override;
    ui::detail::BooksInfo GetAuthorBooks(const std::string& author_id) override;
    ui::detail::BooksInfo GetBooks() override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;

    ui::detail::BooksInfo ConvertBooksToBooksInfo(auto&& callback) {
        ui::detail::BooksInfo books_info;
        auto books = callback();
        for (auto book : books) {
            ui::detail::BookInfo book_info(
                book.GetTitle(),
                book.GetYear().value()
            );
            books_info.push_back(std::move(book_info));
        }
        return books_info;
    }
};

}  // namespace app
