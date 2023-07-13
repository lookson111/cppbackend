#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view src) {
    static constexpr int code_length = 2;
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            if (i+code_length >= src.length())
                throw std::invalid_argument("Error code url not full.");
            [[maybe_unused]] auto s = sscanf(src.substr(i + 1, code_length).data(), "%x", &ii);
	        if (ii < 0x0F || ii > 0xFF)
                throw std::invalid_argument("Error decode url code symbol.");
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + code_length;
        }
        else if (src[i] == '+') {
            ret += ' ';
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
