#include "htmldecode.h"
#include <map>
#include <algorithm>
static const std::map<std::string, char> mp = { 
    {"lt", '<'}, {"LT", '<'},
    {"gt", '>'}, {"GT", '>'}, 
    {"amp", '&'}, {"AMP", '&'}, 
    {"apos", '\''}, {"APOS", '\''}, 
    {"quot", '\"'}, {"QUOT", '\"'}
};

std::string HtmlDecode(std::string_view str) {
    std::string res;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '&') {
            res += '&';
            for (auto m : mp) {
                auto sz = m.first.size();
                if (m.first == str.substr(i+1, sz)) {
                    res.pop_back();
                    res += m.second;
                    i += sz;
                    if (i+1 < str.size() && str[i+1] == ';')
                        i++;
                    break;
                }
            }
        }
        else {
            res += str[i];
        }
    }
    return res;
}
