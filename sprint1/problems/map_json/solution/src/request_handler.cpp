#include "request_handler.h"
#include "iostream"
//#include "boost/property_tree/ptree.hpp"
//#include "boost/property_tree/json_parser.hpp"
#include <boost/json.hpp>
namespace http_handler {
namespace js = boost::json;
bool parse_target(std::string_view target, std::string &map) {
    std::string_view pr = "/api/v1/maps";
    map = "";
    auto pos = target.find(pr);
    if (pos == target.npos)
        return false;
    if (target.size() == pr.size())
        return true;
    map = target.substr(pr.size()+1, target.size() - pr.size());
    //std::cout << "target: " << target << std::endl;
    //std::cout << "map: " << map << std::endl;
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
    //std::cout << serialize(obj) << std::endl;
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
    js::array arr;
    for (auto& r : map->GetRoads()) {
        js::object road;
        road["x0"] = r.GetStart().x;
        road["y0"] = r.GetStart().y;
        if (r.IsHorizontal()) {
            road["x1"] = r.GetEnd().x;
        } else {
            road["y1"] = r.GetEnd().y;
        }
        arr.emplace_back(road);
    }
    mapEl["roads"] = arr;
    arr.clear();
    for (auto& b : map->GetBuildings()) {
        js::object building;
        building["x"] = b.GetBounds().position.x;
        building["y"] = b.GetBounds().position.y;
        building["w"] = b.GetBounds().size.width;
        building["h"] = b.GetBounds().size.height;
        arr.emplace_back(building);
    }
    mapEl["buildings"] = arr;
    arr.clear();
    for (auto& o : map->GetOffices()) {
        js::object office;
        office["id"] = *o.GetId();
        office["x"] = o.GetPosition().x;
        office["y"] = o.GetPosition().y;
        office["offsetX"] = o.GetOffset().dx;
        office["offsetY"] = o.GetOffset().dy;
        arr.emplace_back(office);
    }
    mapEl["offices"] = arr;
    //std::cout << serialize(mapEl) << std::endl;
    return serialize(mapEl);
}
std::string RequestHandler::StatusToJson(std::string_view code, std::string_view message) {
    js::object msg;
    msg["code"] = code.data();
    msg["message"] = message.data();
    //std::cout << serialize(msg) << std::endl;
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
    // Подставьте сюда код из синхронной версии HTTP-сервера
    //return MakeStringResponse(http::status::ok, "OK"sv, req.version(), req.keep_alive());
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
        std::cout << str << std::endl;
        return text_response(stat, str);
    }
    default:
        return text_bad_response(http::status::method_not_allowed);
    }
}
}  // namespace http_handler
