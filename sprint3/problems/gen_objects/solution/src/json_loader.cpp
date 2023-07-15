#include "json_loader.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include <string>
#include <iostream>

namespace json_loader {
using namespace boost::property_tree;
using namespace std::literals;
namespace fs = std::filesystem;

model::Road LoadRoad(ptree &ptreeRoad, model::DDimension road_offset) {
    model::Point start;
    if (ptreeRoad.to_iterator(ptreeRoad.find("y1")) == ptreeRoad.end()) {
        start.x = ptreeRoad.get<int>("x0");
        start.y = ptreeRoad.get<int>("y0");
        return model::Road(model::Road::HORIZONTAL, start, 
            ptreeRoad.get<int>("x1"), road_offset);
    }
    start.x = ptreeRoad.get<int>("x0");
    start.y = ptreeRoad.get<int>("y0");
    return model::Road(model::Road::VERTICAL, start, 
        ptreeRoad.get<int>("y1"), road_offset);
}

model::Building LoadBuilding(ptree &ptB) {
    model::Rectangle r{.position = {.x = ptB.get<int>("x"), .y = ptB.get<int>("y")},
        .size = {.width = ptB.get<int>("w"), .height = ptB.get<int>("h")}};
    return model::Building(r);
}

model::Office LoadOffice(ptree &ptO) {
    model::Office::Id idO{ptO.get<std::string>("id")};
    return model::Office(idO, 
        model::Point{.x = ptO.get<int>("x"), .y = ptO.get<int>("y")}, 
        model::Offset{.dx = ptO.get<int>("offsetX"), .dy = ptO.get<int>("offsetY")});
}

model::Map LoadMap(ptree &ptreeMap, double def_dog_speed) {
    auto id   = ptreeMap.get<std::string>("id");
    auto name = ptreeMap.get<std::string>("name");
    double dog_speed;
    if (ptreeMap.to_iterator(ptreeMap.find("dogSpeed")) != ptreeMap.end()) {
        dog_speed = ptreeMap.get<double>("dogSpeed");
    }
    else {
        dog_speed = def_dog_speed;
    }
    model::Map::Id idmap{id};
    model::Map map(idmap, name, dog_speed);

    ptree jroads = ptreeMap.get_child("roads");
    BOOST_FOREACH(ptree::value_type &jroad, jroads) {
        map.AddRoad(LoadRoad(jroad.second, map.GetRoadOffset()));
    }
    ptree jbuildings = ptreeMap.get_child("buildings");
    BOOST_FOREACH(ptree::value_type &jbuilding, jbuildings) {
        map.AddBuilding(LoadBuilding(jbuilding.second));
    }
    ptree joffices = ptreeMap.get_child("offices");
    BOOST_FOREACH(ptree::value_type &joffice, joffices) {
        map.AddOffice(LoadOffice(joffice.second));
    }
    return map;
}

model::ExtraData LoadExtraData(ptree& ptreeMap) {
    model::ExtraData extra_data;
    auto id = ptreeMap.get<std::string>("id");
    auto loot_types = ptreeMap.get<std::string>("lootTypes");
    size_t cnt = ptreeMap.get_child("lootTypes").size();
    if (cnt < 1)
        throw std::logic_error("The map must contains at least one item!");
    extra_data.SetLootTypes(id, loot_types, static_cast<int>(cnt));
    return extra_data;
}

model::LootGeneratorConfig LoadLootGenConfig(ptree& ptree) {
    static const double sec_to_ms = 1000.0;
    auto loot_conf = ptree.get_child("lootGeneratorConfig");
    model::LootGeneratorConfig lgc;
    lgc.period = std::chrono::milliseconds(static_cast<int>(ptree.get<double>("period")*sec_to_ms));
    lgc.probability = ptree.get<double>("probability");
    return lgc;
}

model::Game LoadGame(const fs::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    ptree pt;
    try {
        read_json(json_path.generic_string(), pt);
    } 
    catch (ptree_error &e) {
        throw std::logic_error("Json file read error: "s + e.what());
    }
    try {
        auto defDogSpeed = pt.get<double>("defaultDogSpeed");
        model::Game game(LoadLootGenConfig(pt));
        ptree jmaps = pt.get_child("maps");
        BOOST_FOREACH(ptree::value_type &jmap, jmaps) {
            game.AddMap(LoadMap(jmap.second, defDogSpeed));
            game.AddExtraData(LoadExtraData(jmap.second));
        }
        return game;
    }
    catch (ptree_error &e) {
        throw std::logic_error("Json parse ptree error: "s + e.what());
    }
}
}  // namespace json_loader
