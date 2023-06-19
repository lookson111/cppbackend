#include "request_handler.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace http_handler {
std::unordered_map<std::string_view, std::string_view> ContentType::type{
    { FE::TEXT_HTML_1,   TEXT_HTML },
    { FE::TEXT_HTML_2,   TEXT_HTML },
    { FE::TEXT_CSS,      TEXT_CSS },
    { FE::TEXT_PLAIN,    TEXT_PLAIN },
    { FE::TEXT_JS,       TEXT_JS },
    { FE::APP_JSON,      APP_JSON },
    { FE::APP_XML,       APP_XML },
    { FE::IMAGE_PNG,     IMAGE_PNG },
    { FE::IMAGE_JPG_1,   IMAGE_JPG },
    { FE::IMAGE_JPG_2,   IMAGE_JPG },
    { FE::IMAGE_JPG_3,   IMAGE_JPG },
    { FE::IMAGE_GIF,     IMAGE_GIF },
    { FE::IMAGE_BMP,     IMAGE_BMP },
    { FE::IMAGE_ICO,     IMAGE_ICO },
    { FE::IMAGE_TIFF_1,  IMAGE_TIFF },
    { FE::IMAGE_TIFF_2,  IMAGE_TIFF },
    { FE::IMAGE_SVG_1,   IMAGE_SVG },
    { FE::IMAGE_SVG_2,   IMAGE_SVG },
    { FE::AUDIO_MP3,     AUDIO_MP3 },
    { FE::EMPTY,         EMPTY },
};

TypeRequest parse_target(std::string_view target, std::string &res) {
    std::string_view api  = "/api/"sv;
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

// Создаёт StringResponse с заданными параметрами
StringResponse RequestHandler::MakeStringResponse(
        http::status status, std::string_view responseText, unsigned http_version,
        bool keep_alive, std::string_view content_type,
        bool no_cache) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (no_cache)
        response.set(http::field::cache_control, "no-cache"sv);
    response.body() = responseText;
    response.content_length(responseText.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse RequestHandler::MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::allow, "GET, HEAD"sv);
    auto ans_v = "Invalid method"sv;
    response.body() = ans_v;
    response.content_length(ans_v.size());
    response.keep_alive(keep_alive);
    return response;
}

VariantResponse RequestHandler::StaticFilesResponse(
        std::string_view target, bool with_body, 
        unsigned http_version, bool keep_alive) {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, http_version, 
            keep_alive, ContentType::TEXT_PLAIN);
    };
    if (target == "/")
        target = "/index.html";
    std::string fullName = static_path_.string() + target.data();
    if (!FileInRootStaticDir(fullName))
        return text_response(http::status::bad_request, "Bad Request");
    if (!CheckFileExist(fullName))
        return text_response(http::status::not_found, "File not found");
    http::file_body::value_type file;
    if (sys::error_code ec; file.open(fullName.c_str(), beast::file_mode::read, ec), ec) {
        return text_response(http::status::gone, 
            app::JsonMessage("Gone"sv, "Failed to open file "s + fullName));
    }
    http::response<http::file_body> res;
    res.version(http_version);
    res.result(http::status::ok);
    std::string ext = fs::path(fullName).extension().string();
    res.set(http::field::content_type, ContentType::get(ext).data());
    
    if (with_body)
        res.body() = std::move(file);
    // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
    // в зависимости от свойств тела сообщения
    res.prepare_payload();
    return std::move(res);
}

VariantResponse RequestHandler::MakeGetResponse(StringRequest& req, bool with_body) {
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

VariantResponse RequestHandler::MakePostResponse(StringRequest& req) {
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

VariantResponse RequestHandler::HandleRequest(StringRequest&& req) {
    // Format response
    try {
        switch (req.method()) {
        case http::verb::get:
            return MakeGetResponse(req, true);
        case http::verb::head:
            return MakeGetResponse(req, false);
        case http::verb::post:
            return MakePostResponse(req);
        default:
            return MakeBadResponse(http::status::method_not_allowed,
                req.version(), req.keep_alive());
        }
    }
    catch (std::exception ex) {
        return MakeStringResponse(http::status::bad_request,
            app::JsonMessage("badRequest", "Server error"s + ex.what()),
            req.version(), req.keep_alive());
    }
}

fs::path RequestHandler::CheckStaticPath(const fs::path& path_static) {
    auto path = fs::weakly_canonical(path_static);
    if (!fs::is_directory(path)) {
	    std::string msgError = "Static path "s + path.generic_string() + 
            " is not exist";
        throw std::invalid_argument(msgError);
    }
    return path;
}
bool RequestHandler::CheckFileExist(std::string &file) const {
    std::string fileName = fs::path(file).filename().string();  
    auto path = fs::path(file).parent_path();
    for (const auto & entry : fs::directory_iterator{path}) {
        std::string fn = entry.path().filename().string();
        if (boost::iequals(fn, fileName)) {
            file = entry.path().string();
            return true;
        }
    }
//    if (fs::exists(filePath))
//        return true;
    return false;
}
bool RequestHandler::FileInRootStaticDir(std::string_view file) const {
    auto path = fs::weakly_canonical(file).string();
    if (path.find(static_path_.string()) == 0)
        return true;
    return false;
}
}  // namespace http_handler
