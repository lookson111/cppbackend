#include "use_cases_impl.h"

#include "../domain/book.h"
#include "../domain/book_tags.h"

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

std::string UseCasesImpl::GetAuthorId(const std::string& name) {
    auto author_id = authors_.GetAuthorId(name);
    return author_id.ToString();
}

void UseCasesImpl::DeleteAuthor(const std::string& author_id) {
    authors_.DeleteAuthor(AuthorId::FromString(author_id));
}

void UseCasesImpl::EditAuthor(ui::detail::AuthorInfo& author_info) {
    authors_.EditAuthor({
        AuthorId::FromString(author_info.id), 
        author_info.name});
}

void UseCasesImpl::AddBook(ui::detail::AddBookParams& book_params) {
    books_.Save({BookId::New(), 
        {AuthorId::FromString(book_params.author_id), ""}, 
        book_params.title,
        book_params.publication_year,
        book_params.tags});
}

void UseCasesImpl::EditBook(ui::detail::BookInfo& book_info) {
    books_.Edit({BookId::FromString(book_info.id), 
        {AuthorId::New(), book_info.author_name}, 
        book_info.title,
        book_info.publication_year,
        book_info.tags});
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

ui::detail::BooksInfo UseCasesImpl::GetBooks(const std::string& start_with) {
    return ConvertBooksToBooksInfo([&](){
        return books_.GetBooks(start_with);
    });
}

ui::detail::BookInfo UseCasesImpl::GetBook(const std::string& book_id) {
    auto book = books_.GetBook(BookId::FromString(book_id));
    return ui::detail::BookInfo(
                book.GetBookId().ToString(),
                book.GetTitle(),
                book.GetAuthor().GetName(),
                book.GetYear().value(),
                book.GetTags()
            );
}

void UseCasesImpl::DeleteBook(const std::string& book_id) {
    books_.DeleteBook(BookId::FromString(book_id));
}

void UseCasesImpl::AddBookTags(ui::detail::BookTagsInfo& book_tags) {
    books_tags_.Save({BookId::FromString(book_tags.book_id), book_tags.tags});
}

}  // namespace app
