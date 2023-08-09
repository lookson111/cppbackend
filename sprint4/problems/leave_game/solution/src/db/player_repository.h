#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include "../player.h"

namespace app {


class RetiredPlayerRepository {
public:
    virtual void Save(const app::RetiredPlayer& dog) = 0;
    virtual void GetRetiredPlayers(app::RetiredPlayers& autors) = 0;

protected:
    ~RetiredPlayerRepository() = default;
};

}  // namespace domain
