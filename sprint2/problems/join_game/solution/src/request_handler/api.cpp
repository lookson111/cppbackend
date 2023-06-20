#include "api.h"


namespace http_handler {

TypeRequest parse_target(std::string_view target, std::string& res) {
    std::string_view api = "/api/"sv;
    std::string_view maps = "/api/v1/maps"sv;
    std::string_view join = "/api/v1/game/join"sv;
    std::string_view players = "/api/v1/game/players"sv;
    res = "";
    std::string uriDecode = http_server::uriDecode(target);
    // request stitic files
    auto pos = target.find(api);
    if (pos == target.npos) {
        res = target;
        return TypeRequest::StaticFiles;
    }
    // finded maps
    pos = target.find(maps);
    if (pos != target.npos) {
        if (target.size() == maps.size())
            return TypeRequest::Maps;
        res = target.substr(maps.size() + 1, target.size() - maps.size());
        return TypeRequest::Map;
    }
    // join game
    pos = target.find(join);
    if (pos != target.npos) {
        return TypeRequest::Join;
    }
    if (target.find(players) != target.npos) {
        return TypeRequest::Players;
    }
    return TypeRequest::None;
}

FileRequestResult Api::MakeGetResponse(const StringRequest& req, bool with_body) const {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
    std::string target;
    // if bad URI
    switch (parse_target(req.target(), target)) {
    case TypeRequest::StaticFiles:
        return StaticFilesResponse(target, with_body, req.version(),
            req.keep_alive());
    case TypeRequest::Maps:
    case TypeRequest::Map: {
        http::status stat;
        auto [text, flag] = app_.GetMapBodyJson(target);
        stat = flag ? http::status::ok : http::status::not_found;
        return text_response(stat, with_body ? text : ""sv);
    }
    case TypeRequest::Join: {
        auto text = app::JsonMessage("invalidArgument"sv,
            "Join game request parse error"sv);
        auto resp = MakeStringResponse(http::status::method_not_allowed, with_body ? text : ""sv,
            req.version(), req.keep_alive(), ContentType::APP_JSON, true);
        resp.set(http::field::allow, "POST"sv);
        return resp;
    }
    case TypeRequest::Players: {
        const auto invalid = [&](std::string_view text) {
            return MakeStringResponse(http::status::unauthorized, text, req.version(), req.keep_alive());
        };
        const auto invalidToken = invalid(app::JsonMessage("invalidToken"sv, "Authorization header is missing"sv));
        std::string token;
        auto it = req.find(http::field::authorization);
        if (it == req.end())
            return invalidToken;
        token = it->value().data();
        auto [body, err] = app_.GetPlayers(token);
        http::status stat;
        switch (err) {
        case app::error_code::InvalidToken:
            return invalidToken;
        case app::error_code::UnknownToken:
            return invalid(app::JsonMessage("unknownToken"sv, "Player token has not been found"sv));
        case app::error_code::None:
            stat = http::status::ok;
            break;
        }
        auto resp = MakeStringResponse(http::status::ok, with_body ? body : ""sv,
            req.version(), req.keep_alive(), ContentType::APP_JSON, true);
        return resp;
    }
    default:
        return text_response(http::status::bad_request,
            with_body ? app::JsonMessage("badRequest", "Bad request") : ""s
        );
    }
}

FileRequestResult Api::MakePostResponse(const StringRequest& req) {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive(),
            ContentType::APP_JSON, true);
    };
    std::string target;
    // if bad URI
    switch (parse_target(req.target(), target)) {
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
    case TypeRequest::Players: {
        auto text = app::JsonMessage("invalidMethod"sv, "Invalid method"sv);
        auto resp = MakeStringResponse(http::status::method_not_allowed, text,
            req.version(), req.keep_alive(), ContentType::APP_JSON, true);
        resp.set(http::field::allow, "GET, HEAD"sv);
        return resp;
    }
    default:
        return text_response(http::status::bad_request,
            app::JsonMessage("badRequest", "Bad request"));
    }
}
}