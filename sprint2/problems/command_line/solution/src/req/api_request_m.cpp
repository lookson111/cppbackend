#include "api_request_m.h"

namespace http_handler
{
    using namespace std::literals;

    void ApiRequestHandler::LinkJoinWithoutAuthorize()
    {
        auto ptr = uri_handler_.AddEndpoint(Endpoint::JOIN_GAME);
        if(ptr)
        {
            ptr->SetNeedAuthorization(false)
                .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
                .SetProcessFunction([&](std::string_view body){
                    return ProcessPostEndpoitWithoutAuthorization(body); } );
        }
    }

    void ApiRequestHandler::LinkPlayersToUriHandler()
    {
        auto ptr = uri_handler_.AddEndpoint(Endpoint::PLAYERS_LIST);
        if(ptr)
        {
            ptr->SetNeedAuthorization(true)
                .SetAllowedMethods({http::verb::get, http::verb::head}, ErrorMessage::GET_IS_EXPECTED, MiscMessage::ALLOWED_GET_HEAD_METHOD)
                .SetProcessFunction([&](const Token& token, std::string_view body){
                    return ProcessGetPlayersListRequest(token, body);
                });
        }
    }
    StringResponse ApiRequestHandler::ProcessPostEndpoitWithoutAuthorization(std::string_view body)
    {
        return Response::Make(http::status::ok, "{1}" );
    }

    StringResponse ApiRequestHandler::ProcessGetPlayersListRequest(const Token& token, std::string_view body)
    {
        if(token != Token("12345678"))
        {
            return Response::MakeUnauthorizedErrorUnknownToken();
        }
        return Response::Make( http::status::ok, "{2}" );
    }
}