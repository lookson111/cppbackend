#include "api.h"
#include "../http_server.h"

namespace http_handler {

TypeRequest Api::ParseTarget(std::string_view target, std::string& res) const {
    std::string_view maps = "/api/v1/maps"sv;
    std::string_view join = "/api/v1/game/join"sv;
    std::string_view players = "/api/v1/game/players"sv;
    std::string_view state   = "/api/v1/game/state"sv;
    std::string_view action  = "/api/v1/game/player/action"sv;
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
    if (uriDecode.find(action) != uriDecode.npos) {
        return TypeRequest::Action;
    }
    return TypeRequest::None;
}
std::string Api::GetToken(const StringRequest& req) const {
    auto it = req.find(http::field::authorization);
    if (it == req.end())
        return "";
    std::string token{it->value()};
    return token;
}

http::status Api::ErrorCodeToStatus(app::error_code ec) const {
    http::status stat = http::status::ok;
    switch (ec) {
    case app::error_code::InvalidToken:
        stat = http::status::unauthorized;
        break;
    case app::error_code::UnknownToken:
        stat = http::status::unauthorized;
        break;
    case app::error_code::None:
        stat = http::status::ok;
        break;
    }
    return stat;
}

std::string Api::CheckToken(std::string_view token) const {
    auto invalidToken = app::JsonMessage("invalidToken"sv, "Authorization header is missing"sv);
    if (token.empty())
        return invalidToken;
    return "";
}

FileRequestResult Api::MakeGetResponse(const StringRequest& req, bool with_body) const {
    const auto body_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, with_body ? text : ""sv, req.version(), 
            req.keep_alive(), ContentType::APP_JSON, true);
    };
    const auto valid_token = [&](std::string_view text) {
        return MakeStringResponse(http::status::unauthorized, text,
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
    case TypeRequest::Players: {
        return ExecuteAuthorized(req, with_body, [&](const std::string& token) {
            return app_.GetPlayers(token);
            });
    }
    case TypeRequest::State: {
        return ExecuteAuthorized(req, with_body, [&](const std::string& token) {
            return app_.GetState(token);
            });
    }
    case TypeRequest::Action:
    case TypeRequest::Join: {
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    }
    default:
        return MakeInvalidMethod(""sv, req.version(), req.keep_alive());
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
    case TypeRequest::Action: {
        return ExecuteAuthorized(req, true, [&](const std::string& token) {
            return app_.MoveAction(token, req.body());
            });
    }
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}

FileRequestResult Api::MakeOptionsResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Action:
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakePutResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Action:
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakePatchResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Action:
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
FileRequestResult Api::MakeDeleteResponse(const StringRequest& req) const
{
    std::string target;
    switch (ParseTarget(req.target(), target)) {
    case TypeRequest::Action:
    case TypeRequest::Join:
        return MakeInvalidMethod("POST"sv, req.version(), req.keep_alive());
    default:
        return MakeInvalidMethod("GET, HEAD"sv, req.version(), req.keep_alive());
    }
}
}
