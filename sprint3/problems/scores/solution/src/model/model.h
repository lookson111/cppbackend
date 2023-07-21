#pragma once 
#include "../sdk.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <atomic>
#include <map>
#include <memory>
#include <list>
#include "../tagged.h"
#include "dog.h"
#include "extra_data.h"
#include "loot_generator.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class RoadRectangle {
    Dimension2D x0, x1, y0, y1;
public:
    RoadRectangle(Point start, Point end, Dimension2D road_offset) noexcept {
        x0 = static_cast<Dimension2D>(std::min(start.x, end.x)) -
            road_offset;
        x1 = static_cast<Dimension2D>(std::max(start.x, end.x)) +
            road_offset;
        y0 = static_cast<Dimension2D>(std::min(start.y, end.y)) -
            road_offset;
        y1 = static_cast<Dimension2D>(std::max(start.y, end.y)) +
            road_offset;
    }
    auto Get() const noexcept {
        return std::make_tuple(x0, x1, y0, y1);
    }
};

class Road {
    struct HorizontalTag {
        HorizontalTag() = default;
    };

    struct VerticalTag {
        VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x, Dimension2D offset) noexcept
        : start_{start}
        , end_{end_x, start.y}
        , road_rectangle_{start_, end_, offset} {
    }

    Road(VerticalTag, Point start, Coord end_y, Dimension2D offset) noexcept
        : start_{start}
        , end_{start.x, end_y} 
        , road_rectangle_{start_, end_, offset} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }
    auto GetRectangle() const noexcept {
        return road_rectangle_.Get();
    }
private:
    Point start_;
    Point end_;
    RoadRectangle road_rectangle_;
};



class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};


struct DefaultMapParam {
    Dimension2D dog_speed;
    size_t bag_capacity;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name, const DefaultMapParam &default_param) noexcept
        : id_(std::move(id)), name_(std::move(name))
        , default_param_(default_param) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(const Office &office);

    Dimension2D GetDogSpeed() const {
        return default_param_.dog_speed;
    }
    Dimension2D GetRoadOffset() const {
        return road_offset_;
    }
    auto GetBagCapacity() const {
        return default_param_.bag_capacity;
    }
private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
    Dimension2D road_offset_ = 0.4;
    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    //DDimension dog_speed_;
    //size_t bag_capacity_;
    DefaultMapParam default_param_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

struct LootGeneratorConfig {
    std::chrono::milliseconds period;
    double probability = 500.0;
};

struct PointKeyHash {
    std::size_t operator()(const Point& k) const {
        return std::hash<Coord>()(k.x) ^ (std::hash<Coord>()(k.y) << 1);
    }
};
struct PointKeyEqual {
    bool operator()(const Point& lhs, const Point& rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

class GameSession {
private:
    using RoadMap = std::unordered_multimap<Point, const Road*, PointKeyHash, PointKeyEqual>;
    using RoadMapIter = decltype(RoadMap{}.equal_range(Point{}));
public:
    using Dogs = std::deque<Dog>;

    GameSession(const Map* map, bool randomize_spawn_points, unsigned cnt_loot_types,
        const LootGeneratorConfig loot_generator_config)
        : map_(map)
        , randomize_spawn_points_(randomize_spawn_points)
        , cnt_loot_types_(cnt_loot_types)
        , loot_generator_(loot_generator_config.period, 
            loot_generator_config.probability,
            [&]() {return GetRandomDouble(0.0, 1.0);}) {
        LoadRoadMap();
    }
    const Map::Id& MapId() {
        return map_->GetId();
    } 
    Dog* FindDog(std::string_view nick_name);
    Dog* AddDog(std::string_view nick_name);
    const Dogs& GetDogs() const {
        return dogs_;
    }
    const Loots& GetLoots() const {
        return loots_;
    }
    void MoveDog(Dog::Id id, Move move);
    void Tick(std::chrono::milliseconds time_delta_ms);

private:
    using DogsIdHasher = util::TaggedHasher<Dog::Id>;
    using DogsIdToIndex = std::unordered_map<Dog::Id, size_t, DogsIdHasher>;
    size_t loot_id_ = 0;
    Dogs dogs_;
    Loots loots_;
    DogsIdToIndex dogs_id_to_index_;
    const Map* map_;
    const bool randomize_spawn_points_ = true;
    RoadMap road_map;
    const unsigned cnt_loot_types_;
    loot_gen::LootGenerator loot_generator_;

    Point2D GetRandomRoadCoord();
    void LoadRoadMap();
    bool PosInRoads(RoadMapIter roads, Point2D pos);
    Point2D GetExtremePos(RoadMapIter roads, Point2D pos);
    Point2D MoveDog(Point2D start_pos, Point2D end_pos);
    static double GetRandomDouble(double min, double max);
    static int GetRandomInt(int min, int max);
    void MoveDogsInMap(std::chrono::milliseconds time_delta_ms);
    void CollectAndReturnLoots();
    void PushLootsToMap(std::chrono::milliseconds time_delta_ms);
};

class Game {
public:
    using Maps = std::vector<Map>;

    Game(const LootGeneratorConfig& loot_generator_config) : loot_generator_config_(std::move(loot_generator_config)){
    }
    void AddMap(const Map& map);
    void AddExtraData(const ExtraData& extra_data) {
        extra_data_ = std::move(extra_data);
    }
    void SetRandomizeSpawnPoints(bool randomize_spawn_points) {
        randomize_spawn_points_ = randomize_spawn_points;
    }
    const Maps& GetMaps() const noexcept {
        return maps_;
    }
    std::string_view GetLootTypes(const Map::Id& id) {
        return extra_data_.GetLootTypes(*id);
    }
    const Map* FindMap(const Map::Id& id) const noexcept;
    GameSession* FindGameSession(const Map::Id& id) noexcept;
    GameSession* AddGameSession(const Map::Id& id);
    void Tick(std::chrono::milliseconds time_delta_ms);

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    
    bool randomize_spawn_points_ = true;
    Maps maps_;
    ExtraData extra_data_;
    const LootGeneratorConfig loot_generator_config_;
    std::deque<GameSession> game_sessions_;
    MapIdToIndex map_id_to_index_;
    MapIdToIndex map_id_to_game_sessions_index_;
};

}  // namespace model
