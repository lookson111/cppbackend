#include "use_cases_impl.h"

#include "../domain/author.h"

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

}  // namespace app
