#include <iostream>
#include "db_books.h"
#include "json_books.h"

using namespace std::literals;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: connect_db <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }
        db_books::BooksDB books_db(argv[1]);
        books_db.CreateTable();
        json_books::JsonBooks json_books{books_db};
        std::string command;
        for (std::string line; std::getline(std::cin, line); ) {
            std::cout << json_books.Command(line) << std::endl;
        }
        json_books.Exit();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
