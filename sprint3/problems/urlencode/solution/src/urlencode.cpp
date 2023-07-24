#include "urlencode.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

std::string UrlEncode(std::string_view src) {
    static constexpr int code_length = 2;
    static constexpr std::string_view reserv_symbs = "!#$&'()*+,/:;=?@[]";
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < src.length(); i++) {
        if (src[i] == '+') {
            ret += ' ';
        } else
        if (src[i] >= 0x80 || src[i] <= 0x20 || reserv_symbs.find(src[i]) != std::string::npos) {
            std::stringstream ss;
            ss << std::setfill('0')
                << std::setw(8)
                << std::hex
                << static_cast<int>(src[i]);
            ret += '\%' + ss.str().substr(6, 2);
        } else {
            ret += src[i];
        }
    }
    return ret;
}