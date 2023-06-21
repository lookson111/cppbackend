#include "app.h"

std::string app::JsonMessage(std::string_view code, std::string_view message) {
    js::object msg;
    msg["code"] = code.data();
    msg["message"] = message.data();
    return serialize(msg);
}

namespace app {
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

std::string ModelToJson::GetMap(std::string_view nameMap) {
    model::Map::Id idmap{nameMap.data()};
    auto map = game_.FindMap({ idmap });
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

std::pair<std::string, bool>
App::GetMapBodyJson(std::string_view mapName) const {
    ModelToJson jmodel(game_);
    std::string body;
    if (mapName.empty()) {
        body = jmodel.GetMaps();
    }
    else {
        // if map not found
        body = jmodel.GetMap(mapName);
        if (!body.size()) {
            return std::make_pair(JsonMessage("mapNotFound", "Map not found"), false);
        }
    }
    return std::make_pair(std::move(body), true);
}
//
std::pair<std::string, JoinError>
App::ResponseJoin(std::string_view jsonBody) {
    auto parseError = std::make_pair(
        JsonMessage("invalidArgument", "Join game request parse error"),
        JoinError::BadJson
    );
    js::error_code ec;
    js::string_view jb{jsonBody.data(), jsonBody.size()};
    std::string resp;
    std::string userName;
    std::string mapId;
    js::value const jv = js::parse(jb, ec);
    if (ec)
        return parseError;
    try {
        userName = jv.at("userName").as_string();
        mapId = jv.at("mapId").as_string();
    }
    catch (const std::system_error&) {
        return parseError;
    }
    catch (const std::invalid_argument&) {
        return parseError;
    }
    if (userName.empty())
        return std::make_pair(
            JsonMessage("invalidArgument", "Invalid name"),
            JoinError::InvalidName
        );
    model::Map::Id idmap{mapId.data()};
    auto map = game_.FindMap({ idmap });
    if (map == nullptr)
        return std::make_pair(
            JsonMessage("mapNotFound", "Map not found"),
            JoinError::MapNotFound
        );
    js::object msg;
    msg["authToken"] = "6516861d89ebfff147bf2eb2b5153ae1";
    msg["plauerId"]  = 0;
    return std::make_pair(
        serialize(msg),
        JoinError::None
    );
}

std::pair<std::string, error_code> App::GetPlayers(std::string_view token) const {
    js::object msg;
    msg["authToken"] = "";
    msg["plauerId"] = 0;
    return std::make_pair(
        serialize(msg),
        error_code::None
    );
}


};
