#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "magic_defs.h"
#include "http_server.h"
#include "token_m.h"
#include "response_m.h"
#include "api_request_m.h"

#include <iostream>
#include <string_view>
#include <string>
#include <boost/json.hpp>
#include <functional>
#include <map>
#include <variant>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class RequestHandler {
public:
    explicit RequestHandler(boost::asio::io_context& io)
        : api_handler_{io} 
	{
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) 
	{
		send(std::move(api_handler_.Handle(req)) );
    }
private:
	ApiRequestHandler api_handler_;
};

}  // namespace http_handler
