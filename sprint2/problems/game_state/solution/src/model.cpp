#include "model.h"
#include <stdexcept>
#include <boost/random.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <ctime>

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
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
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

Dog* GameSession::AddDog(std::string_view nick_name)
{
    if (FindDog(nick_name) != nullptr) {
        throw std::invalid_argument("Duplicate dog");
    }
    DPoint coord = GetRandomRoadCoord();
    Dog dog = Dog(nick_name, coord);
    const size_t index = dogs_.size();
    auto o = dogs_.emplace_back(std::move(dog));
    try {
        dogs_id_to_index_.emplace(o.GetId(), index);
    }
    catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        dogs_.pop_back();
        throw;
    }
    return &dogs_.back();
    return nullptr;
}

DPoint GameSession::GetRandomRoadCoord()
{
    using namespace boost::multiprecision;
    using namespace boost::random;
    auto roads = map_->GetRoads();
    if (roads.size() == 0)
        return DPoint();
    std::time_t now = std::time(0);
    boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
    boost::random::uniform_int_distribution<size_t> dist{0, roads.size()-1};
    independent_bits_engine<mt19937, std::numeric_limits<double>::digits, cpp_int> gen_d;
    auto road = roads[dist(gen)];
    DPoint coord;
    if (road.IsHorizontal()) {
        auto start_point = road.GetStart();
        auto end_point = road.GetEnd();
        uniform_real_distribution<double> ur(start_point.x, end_point.x);
        coord.x = ur(gen_d);
        coord.y = end_point.y;
    }
    else {
        auto start_point = road.GetStart();
        auto end_point = road.GetEnd();
        uniform_real_distribution<double> ur(start_point.y, end_point.y);
        coord.y = ur(gen_d);
        coord.x = end_point.x;
    }
    return coord;
}

Dog* GameSession::FindDog(std::string_view nick_name)
{
    for (auto &dog : dogs_) {
        if (dog.GetName() == nick_name)
            return &dog;
    }
    return nullptr;
}

}  // namespace model
