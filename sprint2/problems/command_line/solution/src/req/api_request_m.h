#ifndef __API_REQUEST_HANDLER_M__
#define __API_REQUEST_HANDLER_M__

#include <optional>
#include <mutex>
#include <boost/asio.hpp>

#include "token_m.h"

#include "response_m.h"
#include "magic_defs.h"
#include "uri_api_m.h"

namespace http_handler
{
    class ApiRequestHandler
    {
    public:
        ApiRequestHandler(boost::asio::io_context& io)
        : uri_handler_()
        {
            ///----------- Register endpoints here ----------------------
            //Add endpoint without authorization and with POST allowed methods
            // LinkJoinWithoutAuthorize();
            //Add players list with GET only and authorization
            // LinkPlayersToUriHandler();
        };

        template <typename Body, typename Allocator>
        bool IsApiRequest(http::request<Body, http::basic_fields<Allocator>>& req)
        {
            return req.target().starts_with(Endpoint::API);
        }

        StringResponse ProcessPostEndpoitWithoutAuthorization(std::string_view body);
        StringResponse ProcessGetPlayersListRequest(const Token& token, std::string_view body);

        template <typename Body, typename Allocator>
        StringResponse ProcessGameRequest(http::request<Body, http::basic_fields<Allocator>>& req)
        {
            return uri_handler_.Process(req);
        }

        template <typename Body, typename Allocator>
        bool IsGameRequest(http::request<Body, http::basic_fields<Allocator>>& req)
        {
            return req.target().starts_with(Endpoint::GAME);
        }

        template <typename Body, typename Allocator>
        auto HandleGameRequest(http::request<Body, http::basic_fields<Allocator>>& req)
        {
            //std::lock_guard gm(access);
            auto response = ProcessGameRequest(req);
            response.keep_alive(req.keep_alive());
            response.version(req.version());
            return response;
        }
        template <typename Body, typename Allocator>
        StringResponse Handle(http::request<Body, http::basic_fields<Allocator>>& req)
        {
            return {std::move(HandleGameRequest(req))};
        }

    private:
        uri_api::UriData uri_handler_;

        void LinkJoinWithoutAuthorize();
        void LinkPlayersToUriHandler();
    };
}
#endif /* __API_REQUEST_HANDLER_M__ */
