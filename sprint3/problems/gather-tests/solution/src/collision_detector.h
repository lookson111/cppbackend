#pragma once

#include "geom.h"

#include <algorithm>
#include <vector>

namespace collision_detector {

struct CollectionResult {
    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
    }
    // квадрат расстояния до точки
    double sq_distance;
    // доля пройденного отрезка
    double proj_ratio;
};

// Движемся из точки a в точку b и пытаемся подобрать точку c.
// Эта функция реализована в уроке.
CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

struct Item {
    geom::Point2D position;
    double width;
    friend bool operator==(const Item& lh, const Item &rh) {
        return lh.position == rh.position && lh.width == rh.width;
    }
};

struct Gatherer {
    geom::Point2D start_pos;
    geom::Point2D end_pos;
    double width;
    friend bool operator==(const Gatherer& lh, const Gatherer &rh) {
        return lh.start_pos == rh.start_pos && lh.end_pos == rh.end_pos && lh.width == rh.width;
    }
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;

public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

class ItemGatherer : ItemGathererProvider {
public:
    using Items = std::vector<Item>;
    using Gatherers = std::vector <Gatherer>;

    void Set(const Gatherers &gatherers);
    void Set(const Items &items);

    virtual size_t ItemsCount() const override;
    virtual Item GetItem(size_t idx) const override;
    virtual size_t GatherersCount() const override;
    virtual Gatherer GetGatherer(size_t idx) const override;

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
};

// Эту функцию вам нужно будет реализовать в соответствующем задании.
// При проверке ваших тестов она не нужна - функция будет линковаться снаружи.
std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

}  // namespace collision_detector