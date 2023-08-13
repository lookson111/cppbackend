#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include "../player.h"

namespace app {


class RetiredPlayerRepository {
public:
    virtual void Save(const app::RetiredPlayer& dog) = 0;
    virtual app::RetiredPlayers Get(uint offset, uint limit) = 0;

protected:
    ~RetiredPlayerRepository() = default;
};

}  // namespace domain
