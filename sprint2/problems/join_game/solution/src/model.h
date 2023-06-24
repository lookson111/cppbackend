#pragma once 
#include "sdk.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <atomic>
#include <map>
#include <memory>
#include "tagged.h"
#include "log.h"

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

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
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

private:
    Point start_;
    Point end_;
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

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
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

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    using Id = util::Tagged<uint64_t, Dog>;
    Dog(std::string_view nickname) : nickname_(nickname.data(), nickname.size()), id_(Id{ idn++ }) {
    }
    const Id& GetId() const {
        return id_;
    }
    std::string_view GetName() const noexcept {
        return nickname_;
    }
    Dog(const Dog& dog) : id_(dog.id_), nickname_(dog.nickname_) {
    }
    Dog(Dog&& dog) noexcept : 
        id_(std::move(dog.id_)),
        nickname_(std::move(dog.nickname_)) {
    }
private:
    static std::atomic<uint64_t> idn;
    Id id_ = Id{0};
    std::string nickname_ = "";
};

class GameSession {
public:
    using Dogs = std::deque<Dog>;
    GameSession(const Map* map) : map_(map) {}
    const Map::Id& MapId() {
        return map_->GetId();
    } 
    Dog* FindDog(std::string_view nick_name);
    Dog* AddDog(std::string_view nick_name);
private:
    using DogsIdHasher = util::TaggedHasher<Dog::Id>;
    using DogsIdToIndex = std::unordered_map<Dog::Id, size_t, DogsIdHasher>;
    Dogs dogs_;
    DogsIdToIndex dogs_id_to_index_;
    const Map* map_;
};

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(const Map& map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }
    const Map* FindMap(const Map::Id& id) const noexcept;
    GameSession* FindGameSession(const Map::Id& id) noexcept;
    GameSession* AddGameSession(const Map::Id& id);

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
     
    Maps maps_;
    std::deque<GameSession> game_sessions_;
    MapIdToIndex map_id_to_index_;
    MapIdToIndex map_id_to_game_sessions_index_;
};

}  // namespace model
