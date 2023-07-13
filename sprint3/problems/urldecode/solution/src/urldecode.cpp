#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            [[maybe_unused]] auto s = sscanf(src.substr(i + 1, 2).data(), "%x", &ii);
	    if (ii < 1 || ii > 0xFF)
                throw std::invalid_argument("Error decode url code symbol.");
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
        else if (src[i] == '+') {
            ret += ' ';
            i = i + 1;
        }
        else if (src[i] >= 'A' && src[i] <= 'Z') {
            ret += src[i] - 'A' + 'a';
        }
        else {
            ret += src[i];
        }
    }
    return ret;
}
