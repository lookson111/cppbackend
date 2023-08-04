#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors)
        : authors_{authors} {
    }

    void AddAuthor(const std::string& name) override;
    void GetAuthors(ui::detail::AuthorsInfo& authors_info) override;
private:
    domain::AuthorRepository& authors_;
};

}  // namespace app
