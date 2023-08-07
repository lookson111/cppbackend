#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"

namespace domain {
/*
namespace detail {
struct DogTag {};
}  // namespace detail

using DogId = util::TaggedUUID<detail::DogTag>;

class Dog {
public:
    Dog(DogId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const DogId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    DogId id_;
    std::string name_;
};

using Dogs = std::vector<Dog>;*/

class DogRepository {
public:
    virtual void Save(const Dog& dog) = 0;
    virtual void GetDogs(Dogs &autors) = 0;
    virtual DogId GetDogId(const std::string& name) = 0;
    virtual void DeleteDog(const DogId& dog_id) = 0;
    virtual void EditDog(const Dog& dog) = 0;

protected:
    ~DogRepository() = default;
};

}  // namespace domain
