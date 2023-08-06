#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include "detail.h"

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool ShowBooks() const;
    bool ShowBook(std::istream& cmd_input) const;
    bool ShowAuthorBooks() const;

    
    std::optional<std::string> SelectAuthor() const;
    std::optional<std::string> EnterAuthor() const;
    std::optional<std::string> GetAuthorId(std::istream& cmd_input) const;
    std::vector<detail::AuthorInfo> GetAuthors() const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::vector<detail::BookInfo> GetBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;

    std::set<std::string> GetBookTags() const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui
