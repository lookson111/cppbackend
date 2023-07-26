#include <boost/serialization/vector.hpp>

#include "model/model.h"

namespace geom {

template <typename Archive>
void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}

template <typename Archive>
void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}

template <typename Archive>
void serialize(Archive& ar, Speed2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}

}  // namespace geom

namespace model {

template <typename Archive>
void serialize(Archive& ar, Loot& obj, [[maybe_unused]] const unsigned version) {
    ar&(*obj.id);
    ar&(obj.type);
    ar&(obj.pos);
}

template <typename Archive>
void serialize(Archive& ar, Loots& obj, [[maybe_unused]] const unsigned version) {
    for (auto &loot : obj)
        ar& (loot);
}
}  // namespace model

namespace serialization {

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , pos_(dog.GetPoint())
        //, bag_capacity_(dog.GetBagCapacity())
        , speed_(dog.GetSpeed())
        , direction_(dog.GetDir())
        , score_(dog.GetScore())
        , bag_content_(dog.GetLoots()) 
    {
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{name_, pos_/*, bag_capacity_*/};
        dog.SetSpeed(speed_);
        dog.SetDirection(direction_);
        dog.AddScore(score_);
        model::Loots bag_content = bag_content_;
        for (auto it = bag_content.begin(), itn = bag_content.begin(); it != bag_content.end(); it = itn) {
            itn = std::next(it);
            dog.PutTheLoot(bag_content, it);
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar& name_;
        ar& pos_;
        //ar& bag_capacity_;
        ar& speed_;
        ar& direction_;
        ar& score_;
        ar& bag_content_;
    }

private:
    model::Dog::Id id_ = model::Dog::Id{0u};
    std::string name_;
    geom::Point2D pos_;
    //size_t bag_capacity_ = 0;
    geom::Speed2D speed_;
    model::Direction direction_ = model::Direction::NORTH;
    model::Score score_ = 0;
    model::Loots bag_content_;
};

/* Другие классы модели сериализуются и десериализуются похожим образом */

}  // namespace serialization
