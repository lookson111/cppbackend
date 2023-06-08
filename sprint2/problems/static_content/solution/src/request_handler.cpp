#include "request_handler.h"
#include "iostream"
namespace http_handler {
bool parse_target(std::string_view target, std::string &map) {
    std::string_view pr = "/api/v1/maps";
    map = "";
    auto pos = target.find(pr);
    if (pos == target.npos)
        return false;
    if (target.size() == pr.size())
        return true;
    map = target.substr(pr.size()+1, target.size() - pr.size());
    return true;
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

js::array ModelToJson::GetRoads(const model::Map::Roads& roads)
{
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

js::array ModelToJson::GetBuildings(const model::Map::Buildings& buildings)
{
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

js::array ModelToJson::GetOffice(const model::Map::Offices& offices)
{
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

std::string RequestHandler::GetMapBodyJson(std::string_view requestTarget, http::status &status) {
    ModelToJson jmodel(game_);
    std::string mapName;
    std::string body;
    // if bad URI
    if (!parse_target(requestTarget, mapName)) {
        status = http::status::bad_request;
        return StatusToJson("badRequest", "Bad request");
    }
    if (mapName.empty()) {
        body = jmodel.GetMaps();
    }
    else {
        body = jmodel.GetMap(mapName);
        // if map not found
        if (!body.size()) {
            status = http::status::not_found;
            return StatusToJson("mapNotFound", "Map not found");
        }
    }
    status = http::status::ok;
    return body;
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

StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    // код из синхронной версии HTTP-сервера
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
    const auto text_bad_response = [&](http::status status) {
        return MakeBadResponse(status, req.version(), req.keep_alive());
    };
    // Format response
    switch (req.method()) {
    case http::verb::get:
    {
        http::status stat;
        auto str = GetMapBodyJson(req.target(), stat);
        return text_response(stat, str);
    }
    default:
        return text_bad_response(http::status::method_not_allowed);
    }
}

}  // namespace http_handler
