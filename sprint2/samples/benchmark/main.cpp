#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time.hpp>
#include "my_logger.h"
using namespace std::literals;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    // чтобы поставить логгеры в равные условия, уберём всё лишнее
    auto ts = rec[timestamp];
    strm << to_iso_extended_string(*ts) << ": ";

    // выводим само сообщение
    strm << rec[expr::smessage];
}

void InitBoostLogFilter() {
    logging::add_common_attributes();

    logging::add_file_log(
        keywords::file_name = "sample.log",
        keywords::format = &MyFormatter
    );
}

void BoostLogIndexInThread(int f, int i) {
    BOOST_LOG_TRIVIAL(info) << "Thread "sv << f << " index "sv << i;
}

class DurationMeasure {
public:
    DurationMeasure() = default;
    ~DurationMeasure() {
        std::chrono::system_clock::time_point end_ts = std::chrono::system_clock::now();
        std::cout << (end_ts - start_ts_).count() << std::endl;
    }

private:
    std::chrono::system_clock::time_point start_ts_ = std::chrono::system_clock::now();
};

int main() {
    InitBoostLogFilter();

    static const int num_iterations = 100000;

    {
        std::cout << "Boost log: "sv << std::flush;
        DurationMeasure g;
        std::thread thread1([](){
            for(int i = 0; i < num_iterations; ++i) {
                BoostLogIndexInThread(1, i);
            }
        });
        std::thread thread2([](){
            for(int i = 0; i < num_iterations; ++i) {
                BoostLogIndexInThread(2, i);
            }
        });
        thread1.join();
        thread2.join();
    }
    {
        std::cout << "Custom log: "sv << std::flush;
        DurationMeasure g;
        std::thread thread1([](){
            for(int i = 0; i < num_iterations; ++i) {
                LogIndexInThread(1, i);
            }
        });
        std::thread thread2([](){
            for(int i = 0; i < num_iterations; ++i) {
                LogIndexInThread(2, i);
            }
        });
        thread1.join();
        thread2.join();
    }
}