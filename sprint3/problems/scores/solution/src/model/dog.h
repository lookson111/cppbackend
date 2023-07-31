#pragma once 
#include "../sdk.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <atomic>
#include <map>
#include <memory>
#include <chrono>
#include "../tagged.h"
#include "geom.h"

namespace model {
using namespace geom;

enum class Move {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    STAND
};

enum class Direction {
    NORTH, 
    SOUTH,
    WEST, 
    EAST
};

struct Loot {
    size_t id;
    int type = 0;
    Point2D pos;
};
using Loots = std::list<Loot>;
using LootParam = int;
using LootsParam = std::vector<LootParam>;

class Dog {
public:
    using Id = util::Tagged<uint64_t, Dog>;
    Dog(std::string_view nickname, const Point2D& coord) :
        nickname_(nickname.data(), nickname.size()), id_(Id{ idn++ }),
        coord_(coord){
    }
    Dog(const Dog& other) : id_(other.id_), nickname_(other.nickname_),
        coord_(other.coord_), speed_(other.speed_), dir_(other.dir_) {
    }
    Dog(Dog&& other) noexcept :
        id_(std::move(other.id_)), nickname_(std::move(other.nickname_)),
        coord_(std::move(other.coord_)), speed_(std::move(other.speed_)),
        dir_(std::move(other.dir_)) {
    }
    const Id& GetId() const {
        return id_;
    }
    std::string_view GetName() const noexcept {
        return nickname_;
    }
    const Point2D& GetPoint() const {
        return coord_;
    }
    const Point2D& GetPrevPoint() const {
        return prev_coord_;
    }
    const Speed2D& GetSpeed() const {
        return speed_;
    }
    std::string GetDirection() const;
    void Diraction(Move move, Dimension2D speed);
    Point2D GetEndPoint(std::chrono::milliseconds move_time_ms);
    void SetPoint(Point2D coord) {
        std::swap(prev_coord_, coord_);
        coord_ = coord;
    }
    bool IsStanding() {
        return speed_ == zero_speed_;
    }
    void Stop() {
        speed_ = zero_speed_;
    }
    void PutTheLoot(Loots &session_loots, auto it_loot) {
        loots_.splice(loots_.end(), session_loots, it_loot);
    }
    const Loots& GetLoots() const {
        return loots_;
    }
    void LootsReturn(const LootsParam& loots_param) {
        for (const auto& loot : loots_) {
            score_ += loots_param[loot.type];
        }
        return loots_.clear();
    }
    int GetScore() const {
        return score_;
    }
private:
    static std::atomic<uint64_t> idn;
    static Speed2D zero_speed_;
    Id id_ = Id{0};
    std::string nickname_ = "";
    Point2D coord_;
    Point2D prev_coord_;
    Speed2D speed_ = zero_speed_;
    Direction dir_ = Direction::NORTH;
    Loots loots_;
    int score_ = 0;
};

}  // namespace model