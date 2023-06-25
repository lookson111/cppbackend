#include "api.h"
#include "../http_server.h"

namespace http_handler {

TypeRequest Api::ParseTarget(std::string_view target, std::string& res) const {
    std::string_view maps = "/api/v1/maps"sv;
    std::string_view join = "/api/v1/game/join"sv;
    std::string_view players = "/api/v1/game/players"sv;
    std::string_view state   = "/api/v1/game/state"sv;
    res = "";
    std::string uriDecode = http_server::uriDecode(target);
    // finded maps
    if (uriDecode.find(maps) != uriDecode.npos) {
        if (uriDecode.size() == maps.size())
            return TypeRequest::Maps;
        res = uriDecode.substr(maps.size() + 1, uriDecode.size() - maps.size());
        return TypeRequest::Map;
    }
    // join game
    if (uriDecode.find(join) != uriDecode.npos) {
        return TypeRequest::Join;
    }
    if (uriDecode.find(players) != uriDecode.npos) {
        return TypeRequest::Players;
    }
    if (uriDecode.find(state) != uriDecode.npos) {
        return TypeRequest::State;
    }
    return TypeRequest::None;
}

std::pair<http::status, std::string> Api::CheckToken(const StringRequest& req) const {
    auto invalidToken = app::JsonMessage("invalidToken"sv, "Authorization header is missing"sv);
    auto it = req.find(http::field::authorization);
    if (it == req.end())
        return std::make_pair(http::status::unauthorized, 
            std::move(invalidToken));//invalidToken);
    std::string token{it->value()};
    auto err = app_.CheckToken(token);
    http::status stat;
    switch (err) {
    case app::error_code::InvalidToken:
        return std::make_pair(http::status::unauthorized,
            std::move(invalidToken));
    case app::error_code::UnknownToken:
        return std::make_pair(http::status::unauthorized, 
            std::move(app::JsonMessage("unknownToken"sv, "Player token has not been found"sv)));
    case app::error_code::None:
        stat = http::status::ok;
        break;
    }
    return std::make_pair(stat, "");
}

FileRequestResult Api::MakeGetResponse(const StringRequest& req, bool with_body) const {
    const auto body_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, with_body ? text : ""sv, req.version(), 
            req.keep_alive(), ContentType::APP_JSON, true);
    };
    const auto valid_token = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text,
            req.version(), req.keep_alive(), ContentType::APP_JSON, true);
    };
    std::string target;
    // if bad URI
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Maps:
    case TypeRequest::Map: {
        http::status stat;
        auto [text, flag] = app_.GetMapBodyJson(target);
        stat = flag ? http::status::ok : http::status::not_found;
        return body_response(stat, text);
    }
    case TypeRequest::Join: {
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    }
    case TypeRequest::Players: {
        auto [status, error_body] = CheckToken(req);
        if (status != http::status::ok)
            return valid_token(status, error_body);
        auto [body, err] = app_.GetPlayers();
        return body_response(status, body);
    }
    case TypeRequest::State: {
        auto [status, error_body] = CheckToken(req);
        if (status != http::status::ok)
            return valid_token(status, error_body);
        auto it = req.find(http::field::authorization);
        std::string token{it->value()};
        auto [body, error_code] = app_.GetState(token);
        return body_response(status, body);
    }
    default:
        return body_response(http::status::bad_request, 
            app::JsonMessage("badRequest", "Get/Head api not found"));
    }
}

FileRequestResult Api::MakePostResponse(const StringRequest& req) const {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive(),
            ContentType::APP_JSON, true);
    };
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Join: {
        auto [body, err] = app_.ResponseJoin(req.body());
        http::status stat;
        switch (err) {
        case app::JoinError::BadJson:
        case app::JoinError::InvalidName:
            stat = http::status::bad_request;
            break;
        case app::JoinError::MapNotFound:
            stat = http::status::not_found;
            break;
        default:
            stat = http::status::ok;
        }
        return text_response(stat, body);

    }
    case TypeRequest::Map:
    case TypeRequest::Maps:
    case TypeRequest::Players: {
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
    default:
        return text_response(http::status::bad_request,
            app::JsonMessage("badRequest", "Post api not found"));
    }
}
FileRequestResult Api::MakeOptionsResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    /*case TypeRequest::Map:
    case TypeRequest::Maps:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    case TypeRequest::Players:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());*/
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakePutResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
   /* case TypeRequest::Map:
    case TypeRequest::Maps:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    case TypeRequest::Players:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());*/
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakePatchResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    /*case TypeRequest::Map:
    case TypeRequest::Maps:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    case TypeRequest::Players:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());*/
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakeDeleteResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    /*case TypeRequest::Map:
    case TypeRequest::Maps:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    case TypeRequest::Players:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());*/
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
}
