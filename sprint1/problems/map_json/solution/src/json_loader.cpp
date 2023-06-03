#include "json_loader.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include <string>
#include <iostream>

namespace json_loader {
using namespace boost::property_tree;
model::Road LoadRoad(ptree &ptreeRoad) {
    model::Point start;
    if (ptreeRoad.to_iterator(ptreeRoad.find("y1")) == ptreeRoad.end()) {
        start.x = ptreeRoad.get<int>("x0");
        start.y = ptreeRoad.get<int>("y0");
        return model::Road(model::Road::HORIZONTAL, start, 
            ptreeRoad.get<int>("x1"));
    }
    start.x = ptreeRoad.get<int>("x0");
    start.y = ptreeRoad.get<int>("y0");
    return model::Road(model::Road::VERTICAL, start, 
        ptreeRoad.get<int>("y1"));
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

model::Map LoadMap(ptree &ptreeMap) {
    auto id   = ptreeMap.get<std::string>("id");
    auto name = ptreeMap.get<std::string>("name");
    model::Map::Id idmap{id};
    model::Map map(idmap, name);
    ptree jroads = ptreeMap.get_child("roads");
    BOOST_FOREACH(ptree::value_type &jroad, jroads) {
        map.AddRoad(LoadRoad(jroad.second));
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

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;
    model::Office office();
    ptree pt;
    try {
        read_json(json_path, pt);
    } 
    catch (ptree_error &e) {
        std::cout << "Json file read error: " << e.what();
        throw;
    }
    try {
        ptree jmaps = pt.get_child("maps");
        BOOST_FOREACH(ptree::value_type &jmap, jmaps) {
            game.AddMap(LoadMap(jmap.second));
        }
    }
    catch (ptree_error &e) {
        std::cout << "Json parse ptree error: " << e.what();
        throw;
    }
    return game;
}
}  // namespace json_loader
