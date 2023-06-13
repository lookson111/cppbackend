#include "log.h"
#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр 
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/date_time.hpp>
#include <string_view>

namespace Logger {
using namespace std::literals;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int)
BOOST_LOG_ATTRIBUTE_KEYWORD(data, "Data", std::string_view)
BOOST_LOG_ATTRIBUTE_KEYWORD(msg, "Msg", std::string_view)

//{"timestamp":"2022-09-17T01:04:47.841269", "data" : {"port":8080, "address" : "0.0.0.0"}, "message" : "server started"}
//{"timestamp":"2022-09-17T01:04:59.502760", "data" : {"ip":"127.0.0.1", "URI" : "/", "method" : "GET"}, "message" : "request received"}
//{"timestamp":"2022-09-17T01:04:59.506669", "data" : {"ip":"127.0.0.1", "response_time" : 3, "code" : 200, "content_type" : "text/html"}, "message" : "response sent"}
//{"timestamp":"2022-09-17T01:04:59.559406", "data" : {"ip":"127.0.0.1", "URI" : "/images/cube.svg", "method" : "GET"}, "message" : "request received"}
//{"timestamp":"2022-09-17T01:04:59.563314", "data" : {"ip":"127.0.0.1", "response_time" : 4, "code" : 200, "content_type" : "image/svg+xml"}, "message" : "response sent"}
//{"timestamp":"2022-09-17T01:05:13.894737", "data" : {"ip":"127.0.0.1", "URI" : "/file%20with+spaces.html", "method" : "GET"}, "message" : "request received"}
//{"timestamp":"2022-09-17T01:05:13.898645", "data" : {"ip":"127.0.0.1", "response_time" : 3, "code" : 200, "content_type" : "text/html"}, "message" : "response sent"}
//{"timestamp":"2022-09-17T01:05:23.092869", "data" : {"ip":"127.0.0.1", "URI" : "/abracadabra", "method" : "GET"}, "message" : "request received"}
//{"timestamp":"2022-09-17T01:05:23.102635", "data" : {"ip":"127.0.0.1", "response_time" : 9, "code" : 404, "content_type" : "text/html"}, "message" : "response sent"}
//{"timestamp":"2022-09-17T01:05:29.502651", "data" : {"code":1, "text" : "The socket was closed due to a timeout", "where" : "read"}, "message" : "error"}
//{"timestamp":"2022-09-17T01:05:29.746568", "data" : {"code":1, "text" : "The socket was closed due to a timeout", "where" : "read"}, "message" : "error"}
//{"timestamp":"2022-09-17T01:05:51.777199", "data" : {"code":0}, "message" : "server exited"}


static void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    // Момент времени приходится вручную конвертировать в строку.
    // Для получения истинного значения атрибута нужно добавить
    // разыменование. 
    auto ts = *rec[timestamp];
    strm << "{\"timestamp\":\"" << to_iso_extended_string(ts) << "\", ";
    strm << "\"data\" : " << rec[data] << ", ";
    strm << "\"message\" : \"" << rec[msg] << "\"}";
}

void InitBoostLogFilter() {
    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info
    );
    logging::add_common_attributes();
    logging::add_file_log(
        keywords::file_name = "sample_%N.log",
        keywords::format = &JsonFormatter,
        keywords::open_mode = std::ios_base::app | std::ios_base::out,
        // ротируем по достижению размера 10 мегабайт
        keywords::rotation_size = 10 * 1024 * 1024,
        // ротируем ежедневно в полдень
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)
    );
    logging::add_console_log(
        std::clog,
        keywords::format = &JsonFormatter,
        keywords::auto_flush = true
    );
}

void Log::info(std::string_view data_, std::string_view message_) {
    BOOST_LOG_TRIVIAL(info)
        << logging::add_value(data, data_)
        << logging::add_value(msg, message_);
}

}