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
#include "../util/tagged_uuid.h"
#include "geom.h"

namespace model {
using namespace geom;
using milliseconds = std::chrono::milliseconds;

using LootParam = int;
using LootsParam = std::vector<LootParam>;
using Score = unsigned;
using LostObjectType = unsigned;

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
    using Id = util::Tagged<uint32_t, Loot>;

    Id id{ 0u };
    LostObjectType type{ 0u };
    Point2D pos;

    [[nodiscard]] auto operator<=>(const Loot&) const = default;
};
using Loots = std::list<Loot>;

class Dog {
public:
    using Id = util::Tagged<uint64_t, Dog>;

    Dog(Id id, std::string nickname, Point2D coord) :
        nickname_(std::move(nickname)), id_(std::move(id)),
        coord_(std::move(coord)){
    }
    Dog(const Dog& other) 
        : id_(other.id_)
        , nickname_(other.nickname_)
        , coord_(other.coord_)
        , prev_coord_(other.prev_coord_)
        , speed_(other.speed_)
        , dir_(other.dir_)
        , loots_(other.loots_)
        , score_(other.score_)
    {}
    Dog(Dog&& other) noexcept 
        : id_(std::move(other.id_))
        , nickname_(std::move(other.nickname_))
        , coord_(std::move(other.coord_))
        , prev_coord_(std::move(other.prev_coord_))
        , speed_(std::move(other.speed_))
        , dir_(std::move(other.dir_))
        , loots_(std::move(other.loots_))
        , score_(std::move(other.score_))
    {}
    Dog &operator=(const Dog& other) {
        if (this == &other)
            return *this;
        id_         = other.id_;
        nickname_   = other.nickname_;
        coord_      = other.coord_;
        prev_coord_ = other.prev_coord_;
        speed_      = other.speed_;
        dir_        = other.dir_;
        loots_      = other.loots_;
        score_      = other.score_;
        return *this;
    }
    const Id& GetId() const {
        return id_;
    }
    const std::string& GetName() const noexcept {
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
    Direction GetDir() const {
        return dir_;
    }
    void Diraction(Move move, Dimension2D speed);
    Point2D GetEndPoint(milliseconds move_time_ms);
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
    Score GetScore() const {
        return score_;
    }
    void SetSpeed(const Speed2D& speed) {
        speed_ = speed;
    }
    void SetDirection(Direction dir) {
        dir_ = dir;
    }
    void AddScore(Score score) {
        score_ = score;
    }
    milliseconds GetStayTime() const {
        return lifetime_-last_move_time_;
    }
    milliseconds GetLifetime() const {
        return lifetime_;
    }
    void IncLifetime(milliseconds time_delta_ms) {
        lifetime_ += time_delta_ms;
        if (speed_ != zero_speed_) {
            last_move_time_ = lifetime_;
        }
    }
private:
    static Speed2D zero_speed_;
    Id id_ = Id{0};
    std::string nickname_ = "";
    Point2D coord_;
    Point2D prev_coord_;
    Speed2D speed_ = zero_speed_;
    Direction dir_ = Direction::NORTH;
    Loots loots_; 
    Score score_ = 0;
    milliseconds last_move_time_;
    milliseconds lifetime_;    
};

}  // namespace model
