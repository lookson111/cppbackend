#ifdef WIN32
#include <sdkddkver.h>
#endif

//#include <boost/asio.hpp>
//#include <chrono>
#include <syncstream>
//#include "hamburger.h"
//namespace net = boost::asio;
//namespace sys = boost::system;
//using namespace std::chrono;

class Logger {
public:
    explicit Logger(std::string id)
        : id_(std::move(id)) {
    }

    void LogMessage(std::string_view message) const {
        std::osyncstream os{std::cout};
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
           << "s] "sv << message << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
}; 
int main() {
    return 0;
}
