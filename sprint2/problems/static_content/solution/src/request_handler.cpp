#include "request_handler.h"
#include <iostream>

namespace http_handler {
std::unordered_map<std::string_view, std::string_view> ContentType::type{
    { FE_TEXT_HTML_1,   TEXT_HTML },
    { FE_TEXT_HTML_2,   TEXT_HTML },
    { FE_TEXT_CSS,      TEXT_CSS },
    { FE_TEXT_PLAIN,    TEXT_PLAIN },
    { FE_TEXT_JS,       TEXT_JS },
    { FE_APP_JSON,      APP_JSON },
    { FE_APP_XML,       APP_XML },
    { FE_IMAGE_PNG,     IMAGE_PNG },
    { FE_IMAGE_JPG_1,   IMAGE_JPG },
    { FE_IMAGE_JPG_2,   IMAGE_JPG },
    { FE_IMAGE_JPG_3,   IMAGE_JPG },
    { FE_IMAGE_GIF,     IMAGE_GIF },
    { FE_IMAGE_BMP,     IMAGE_BMP },
    { FE_IMAGE_ICO,     IMAGE_ICO },
    { FE_IMAGE_TIFF_1,  IMAGE_TIFF },
    { FE_IMAGE_TIFF_2,  IMAGE_TIFF },
    { FE_IMAGE_SVG_1,   IMAGE_SVG },
    { FE_IMAGE_SVG_2,   IMAGE_SVG },
    { FE_AUDIO_MP3,     AUDIO_MP3 },
    { FE_EMPTY,         EMPTY },
};

std::string urlDecode(std::string_view src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            [[maybe_unused]] auto s = sscanf_s(src.substr(i + 1, 2).data(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
        else if (src[i] == '+') {
            ret += ' ';
            i = i + 1;
        }
        else {
            ret += src[i];
        }
    }
    return (ret);
}

TypeRequest parse_target(std::string_view target, std::string &res) {
    std::string_view api = "/api/"sv;
    std::string_view pr = "/api/v1/maps"sv;
    res = "";
    auto pos = target.find(api);
    if (pos == target.npos) {
        // request stitic files
        res = target;
        return TypeRequest::StaticFiles;
    }
    pos = target.find(pr);
    if (pos == target.npos)
        return TypeRequest::None;
    if (target.size() == pr.size())
        return TypeRequest::Maps;
    res = target.substr(pr.size() + 1, target.size() - pr.size());
    return TypeRequest::Map;
}

std::string ModelToJson::GetMaps() {
    const auto& maps = game_.GetMaps();
    js::array obj;
    for (auto& map : maps) {
        js::object mapEl;
        mapEl["id"] = *map.GetId();
        mapEl["name"] = map.GetName();
        obj.emplace_back(mapEl);
    }
    return serialize(obj);
}

std::string ModelToJson::GetMap(std::string nameMap) {
    model::Map::Id idmap{nameMap};
    auto map = game_.FindMap({idmap});
    if (map == nullptr)
        return "";
    js::object mapEl;
    mapEl["id"] = *map->GetId();
    mapEl["name"] = map->GetName();
    mapEl["roads"] = GetRoads(map->GetRoads());
    mapEl["buildings"] = GetBuildings(map->GetBuildings());
    mapEl["offices"] = GetOffice(map->GetOffices());
    return serialize(mapEl);
}

js::array ModelToJson::GetRoads(const model::Map::Roads& roads) {
    js::array arr;
    for (auto& r : roads) {
        js::object road;
        road["x0"] = r.GetStart().x;
        road["y0"] = r.GetStart().y;
        if (r.IsHorizontal()) {
            road["x1"] = r.GetEnd().x;
        }
        else {
            road["y1"] = r.GetEnd().y;
        }
        arr.emplace_back(std::move(road));
    }
    return arr;
}

js::array ModelToJson::GetBuildings(const model::Map::Buildings& buildings) {
    js::array arr;
    for (auto& b : buildings) {
        js::object building;
        building["x"] = b.GetBounds().position.x;
        building["y"] = b.GetBounds().position.y;
        building["w"] = b.GetBounds().size.width;
        building["h"] = b.GetBounds().size.height;
        arr.emplace_back(std::move(building));
    }
    return arr;
}

js::array ModelToJson::GetOffice(const model::Map::Offices& offices) {
    js::array arr;
    for (auto& o : offices) {
        js::object office;
        office["id"] = *o.GetId();
        office["x"] = o.GetPosition().x;
        office["y"] = o.GetPosition().y;
        office["offsetX"] = o.GetOffset().dx;
        office["offsetY"] = o.GetOffset().dy;
        arr.emplace_back(std::move(office));
    }
    return arr;
}

std::string RequestHandler::StatusToJson(std::string_view code, std::string_view message) {
    js::object msg;
    msg["code"] = code.data();
    msg["message"] = message.data();
    return serialize(msg);
}

std::pair<std::string, http::status> 
RequestHandler::GetMapBodyJson(std::string_view requestTarget) {
    ModelToJson jmodel(game_);
    std::string mapName;
    std::string body;
    http::status status;
    if (mapName.empty()) {
        body = jmodel.GetMaps();
    }
    else {
        body = jmodel.GetMap(mapName);
        // if map not found
        if (!body.size()) {
            status = http::status::not_found;
            return std::make_pair(StatusToJson("mapNotFound", "Map not found"), status);
        }
    }
    status = http::status::ok;
    return std::make_pair(body, status);
}

// Создаёт StringResponse с заданными параметрами
StringResponse RequestHandler::MakeStringResponse(
        http::status status, std::string_view responseText, unsigned http_version,
        bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
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

StringResponse RequestHandler::StaticFilesResponse(
        std::string_view target, unsigned http_version,
        bool keep_alive) {
    if (!CheckFileExist(target))
        MakeStringResponse(http::status::not_found,
            "File not found", http_version, keep_alive,
            ContentType::TEXT_PLAIN);
    if (!FileInRootStaticDir(target))
        MakeStringResponse(http::status::bad_request,
            "Bad Request", http_version, keep_alive,
            ContentType::TEXT_PLAIN);

    http::status status = http::status::ok;
    std::string_view content_type = ContentType::TEXT_PLAIN;
    
    http::response<http::file_body> res;
    res.version(http_version);
    res.result(http::status::ok);
    res.insert(http::field::content_type, "text/plain"sv);

    http::file_body::value_type file;
    if (sys::error_code ec; file.open(target.data(), beast::file_mode::read, ec), ec) {
        std::cout << "Failed to open file "sv << target.data() << std::endl;
        return MakeStringResponse(http::status::gone,
            "Failed to open file "s + target.data(), 
            http_version, keep_alive,
            ContentType::TEXT_PLAIN);
    }

    res.body() = std::move(file);
    // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
    // в зависимости от свойств тела сообщения
    res.prepare_payload();
    res.set(http::field::content_type, content_type);


    StringResponse response(status, http_version);
    return response;
}

StringResponse RequestHandler::StaticFilesHeadResponse(
        std::string_view target, unsigned http_version,
        bool keep_alive) {
    http::status status = http::status::ok;
    std::string_view content_type = ContentType::TEXT_PLAIN;

    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    return response;
}

StringResponse RequestHandler::MakeGetResponse(StringRequest& req) {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
    std::string target;
    std::string uriDecode = urlDecode(req.target());
    // if bad URI
    switch (parse_target(uriDecode, target)) {
    case TypeRequest::Maps:
    case TypeRequest::Map:
    {
        auto [text, stat] = GetMapBodyJson(target);
        return text_response(stat, text);
    }
    case TypeRequest::StaticFiles:
        std::cout << "StaticFiles" << std::endl;
        return StaticFilesResponse(target, req.version(), 
            req.keep_alive());
    default:
        return text_response(http::status::bad_request,
            StatusToJson("badRequest", "Bad request"));
    }
}

StringResponse RequestHandler::MakeHeadResponse(StringRequest& req) {
    std::string target;
    std::string uriDecode = urlDecode(req.target());
    // if bad URI
    switch (parse_target(req.target(), target)) {
    case TypeRequest::StaticFiles:
        return StaticFilesHeadResponse(target, req.version(), 
            req.keep_alive());
    default:
        return MakeStringResponse(http::status::bad_request,
            StatusToJson("badRequest", "Bad request"),
            req.version(), req.keep_alive());
    }
}

StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    std::cout << "Target code  : " << req.target() << std::endl;
    std::cout << "Target decode: " << urlDecode(req.target()) << std::endl;
    // Format response
    switch (req.method()) {
    case http::verb::get:
        std::cout << "GET" << std::endl;
        return MakeGetResponse(req);
    case http::verb::head:
        std::cout << "HEAD" << std::endl;
        return MakeHeadResponse(req);
    default:
        return MakeBadResponse(http::status::method_not_allowed, 
            req.version(), req.keep_alive());
    }
}

fs::path RequestHandler::CheckStaticPath(const fs::path& path_static) {
    auto path = fs::weakly_canonical(path_static);
    if (!fs::is_directory(path)) {
        auto msgError = "Static path "s + path.generic_string() + " is not exist";
        throw std::exception(msgError.c_str());
    }
    std::cout << path.generic_string() << std::endl;
    return path;
}
bool RequestHandler::CheckFileExist(std::string_view file) {
    fs::path filePath(file);
    if (fs::exists(filePath))
        return true;
    return false;
}
bool RequestHandler::FileInRootStaticDir(std::string_view file) const {
    std::string f = static_path_.string() + file.data();
    auto path = fs::weakly_canonical(f).string();
    if (path.find(f) == 0)
        return true;
    return false;
}
}  // namespace http_handler