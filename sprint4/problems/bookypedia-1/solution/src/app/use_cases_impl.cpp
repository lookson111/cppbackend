#include "use_cases_impl.h"

#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}
void UseCasesImpl::GetAuthors(ui::detail::AuthorsInfo& authors_info) {
    domain::Authors authors;
    authors_.GetAuthors(authors);
    for (auto& author : authors) {
        ui::detail::AuthorInfo author_info(author.GetId().ToString(), 
            author.GetName());
        authors_info.push_back(std::move(author_info));
    }
}
void UseCasesImpl::AddBook(ui::detail::AddBookParams& book_params) {
    books_.Save({BookId::New(), 
        AuthorId::FromString(book_params.author_id), 
        book_params.title,
        book_params.publication_year});
}
ui::detail::BooksInfo UseCasesImpl::GetAuthorBooks(const std::string& author_id) {
    return ConvertBooksToBooksInfo([&](){
        return books_.GetAuthorBooks(AuthorId::FromString(author_id));
    });
}
ui::detail::BooksInfo UseCasesImpl::GetBooks() {
    return ConvertBooksToBooksInfo([&](){
        return books_.GetBooks();
    });
}

}  // namespace app
