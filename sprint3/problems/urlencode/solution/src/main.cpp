#include <iostream>
#include <sstream>
#include <iomanip>

#include "urlencode.h"

int main() {
    std::string s;
    std::getline(std::cin, s);
    std::cout << UrlEncode(s) << std::endl;
}
