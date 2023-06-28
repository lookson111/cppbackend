#include "model.h"
#include <stdexcept>
#include <boost/random.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <ctime>
#include <random>

#define MAX_ROADS_TO_FOUND 100

namespace model {
using namespace std::literals;

void Map::AddOffice(const Office &office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }
    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_mapS
        offices_.pop_back();
        throw;
    }
}

void Game::AddMap(const Map &map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

GameSession* Game::FindGameSession(const Map::Id& id) noexcept {
    if (auto it = map_id_to_game_sessions_index_.find(id);
        it != map_id_to_game_sessions_index_.end()) {
        return &game_sessions_.at(it->second);
    }
    return nullptr;
}

GameSession* Game::AddGameSession(const Map::Id& id) {
    GameSession gs{ FindMap(id) };
    const size_t index = game_sessions_.size();
    game_sessions_.emplace_back(std::move(gs));
    try {
        map_id_to_game_sessions_index_.emplace(id, index);
    }
    catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        game_sessions_.pop_back();
        throw;
    }
    return &game_sessions_.back();
}

void Game::Tick(uint64_t time_delta_ms) {
    for (auto &game_session : game_sessions_) {
        game_session.Tick(time_delta_ms);
    }
}

Dog* GameSession::AddDog(std::string_view nick_name)
{
    if (FindDog(nick_name) != nullptr) {
        throw std::invalid_argument("Duplicate dog");
    }
    DPoint coord = GetRandomRoadCoord();
    Dog dog = Dog(nick_name, coord);
    const size_t index = dogs_.size();
    auto &o = dogs_.emplace_back(std::move(dog));
    try {
        dogs_id_to_index_.emplace(o.GetId(), index);
    }
    catch (...) {
        dogs_.pop_back();
        throw;
    }
    return &dogs_.back();
    return nullptr;
}

DPoint GameSession::GetRandomRoadCoord()
{
    std::time_t now = std::time(0);
    boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
    auto random_double = [&](auto x1, auto x2) {
        if (x2 < x1) 
            std::swap(x1, x2);
        std::uniform_real_distribution<double> ur(x1, x2);
        return ur(gen);
    };
    auto &roads = map_->GetRoads();
    if (roads.size() == 0)
        return DPoint();
    boost::random::uniform_int_distribution<size_t> dist{0, roads.size()-1};
    auto &road = roads[dist(gen)];
    DPoint coord;
    if (road.IsHorizontal()) {
        auto start_point = road.GetStart();
        auto end_point = road.GetEnd();
        coord.x = random_double(start_point.x, end_point.x);
        coord.y = end_point.y;
    }
    else {
        auto start_point = road.GetStart();
        auto end_point = road.GetEnd();
        coord.y = random_double(start_point.y, end_point.y);
        coord.x = end_point.x;
    }
    return coord;
}

void GameSession::LoadRoadMap() {
    const auto &roads = map_->GetRoads();
    for (const auto& road : roads) {
        auto start = road.GetStart();
        auto end = road.GetEnd();
        if (road.IsHorizontal()) {
            for (auto x = start.x; x <= end.x; x++) {
                road_map.emplace(Point{.x = x, .y = start.y}, &road);
            }
        }
        if (road.IsVertical()) {
            for (auto y = start.y; y <= end.y; y++) {
                road_map.emplace(Point{ .x = start.x, .y = y }, &road);
            }
        }
    }
}

Dog* GameSession::FindDog(std::string_view nick_name)
{
    for (auto &dog : dogs_) {
        if (dog.GetName() == nick_name)
            return &dog;
    }
    return nullptr;
}

void GameSession::MoveDog(Dog::Id id, Move move) {
    auto& dog = dogs_[dogs_id_to_index_[id]];
    dog.Diraction(move, map_->GetDogSpeed());
}

void GameSession::Tick(uint64_t time_delta_ms)
{
    auto pos_round = [](auto &pos) {
        Point point_pos{ .x = static_cast<Coord>(std::round(pos.x)),
            .y = static_cast<Coord>(std::round(pos.y))};
        return point_pos;
    };
    for (auto& dog : dogs_) {
        auto start_pos = dog.GetPoint();
        auto end_pos = dog.GetEndPoint(time_delta_ms);
        bool is_vertical = start_pos.y == end_pos.y;
        if (start_pos == end_pos)
            continue;
        //Point dog_pos{ .x = static_cast<Coord>(std::round(start_pos.x)), 
        //    .y = static_cast<Coord>(std::round(start_pos.y))};
        Point dog_cell = pos_round(start_pos);
        auto prev_cell = start_pos;
        int protect = 0;
        do {
            auto roads = road_map.equal_range(dog_cell);
            if (PosInRoads(roads, end_pos)) {
                dog.SetPoint(end_pos);
                continue;
            }
            auto extreme_pos = GetExtremePos(roads, end_pos);
            //prev_cell = dog_cell;
            dog_cell = pos_round(extreme_pos);
            prev_pos = start_pos;
            start_pos = extreme_pos;
            protect++;
            if (protect >= MAX_ROADS_TO_FOUND)
                throw std::exception("Error, not found end cell in roads");
        } while (prev_pos != extreme_pos);
        // TODO if the dog is on the edge, it is necessery to stop him

        //for (auto it = roads.first; it != roads.second; ++it) {
        // }
    }
}
}  // namespace model
