#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <boost/regex.hpp>
using namespace std::literals;
int GetIntParam(const std::string& params, const std::string& name, int def_val) {
    int res = def_val;
    boost::regex expr{"(\\?|&|^|,)"s + name + "=(\\w+)(\\&|$)"s };
    boost::smatch what;
    if (boost::regex_search(params, what, expr)) {
        std::cout << what[2] << '\n';
        res = std::stoi(what[2]);
    }
    return res;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {
    std::string str = "?score=1&time=13";
    int score = GetIntParam(str, "score", 0);
    int time =  GetIntParam(str, "time", 0);
    return 0;
}
