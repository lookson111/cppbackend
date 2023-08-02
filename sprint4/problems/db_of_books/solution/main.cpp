#include <iostream>
#include <pqxx/pqxx>

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: connect_db <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
