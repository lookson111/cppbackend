#include "infrastructure.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <filesystem>
#include "../log.h"

namespace infrastructure {
using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

SerializingListiner::SerializingListiner(model::Game &game, 
        const std::string& state_file, milliseconds save_period) 
    : game_(game)
    , state_file_(state_file)
    , save_period_(save_period) 
{
    time_since_save_ = 0ms;
    if (std::filesystem::exists(state_file))
        Load();
}
void SerializingListiner::OnTick(milliseconds time_delta_ms) {
    using namespace std::chrono_literals;
    time_since_save_ += time_delta_ms;
    if (time_since_save_ >= save_period_) {
        LOGSRV().Msg("Tick", "serializing "s + std::to_string(time_since_save_.count()) +
            " out"s + std::to_string(save_period_.count()));
        Save();
        time_since_save_ = 0ms;
    }
}

void SerializingListiner::Load() {
    std::ifstream archive_{state_file_};
    InputArchive input_archive{archive_};
    serialization::GameRepr repr(game_);
    input_archive >> repr;
    repr.Restore(game_);
}

void SerializingListiner::Save() {
    std::ofstream archive_{state_file_};
    OutputArchive output_archive{archive_};
    output_archive << serialization::GameRepr{game_};
}

} // namespace infrastructure
