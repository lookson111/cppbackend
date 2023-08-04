#pragma once

#include <string>
#include "../ui/detail.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void GetAuthors(ui::detail::AuthorsInfo& authors_info) = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
