#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
private:
    static const size_t shot_result_sz = 1;
    static const size_t shot_coord_sz = 2;
    using type_readExact = std::optional<std::string>;
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        // TODO: реализуйте самостоятельно
        try {
            while (!IsGameEnded()) {
                PrintFields();
                if (my_initiative) {
                    std::string move = make_move();
                    auto move_pos = ParseMove(move);
                    if (move_pos == std::nullopt) {
                        std::cout << "Incorrect move!" << std::endl;
                        continue;
                    }
                    // send shot
                    if (!WriteExact(socket, move)) {
                        std::cout << "Error connection!";
                        return;
                    }
                    // get shot
                    auto short_result_str = ReadExact<shot_result_sz>(socket);
                    if (!short_result_str) {
                        std::cout << "Error connection!";
                        return;
                    }
                    auto short_result = stringToShorRes(*short_result_str);
                    if (!short_result) {
                        std::cout << "Incorrect short result!";
                        continue;
                    }
                    switch (*short_result) {
                    case SeabattleField::ShotResult::HIT:
                        std::cout << "Hit" << std::endl;
                        other_field_.MarkHit(move_pos->second, move_pos->first);
                        break;
                    case SeabattleField::ShotResult::KILL:
                        std::cout << "Kill" << std::endl;
                        other_field_.MarkKill(move_pos->second, move_pos->first);
                        break;
                    case SeabattleField::ShotResult::MISS:
                        std::cout << "Miss" << std::endl;
                        other_field_.MarkMiss(move_pos->second, move_pos->first);
                        my_initiative = false;
                        break;
                    }
                }
                else {
                    // get shot
                    auto short_str = ReadExact<shot_coord_sz>(socket);
                    if (!short_str) {
                        std::cout << "Error connection!";
                        return;
                    }
                    std::string_view sv = *short_str;
                    auto shot_pos = ParseMove(sv);
                    if (!shot_pos) {
                        std::cout << "Incorrect move!" << std::endl;
                        continue;
                    }
                    auto shot_res = my_field_.Shoot(shot_pos->second, shot_pos->first);
                    switch (shot_res) {
                    case SeabattleField::ShotResult::HIT:
                        std::cout << "Hit" << std::endl;
                        break;
                    case SeabattleField::ShotResult::KILL:
                        std::cout << "Kill" << std::endl;
                        break;
                    case SeabattleField::ShotResult::MISS:
                        std::cout << "Miss" << std::endl;
                        my_initiative = true;
                        break;
                    }
                    //my_initiative = how_initiative(shot_res, my_initiative);
                    // send shot result
                    if (!WriteExact(socket, std::to_string(static_cast<char>(shot_res)))) {
                        std::cout << "Error connection!";
                        return;
                    }
                }
            }
            if (my_field_.IsLoser())
                std::cout << "Game over!" << std::endl;
            else
                std::cout << "Victory!" << std::endl;
        }
        catch (std::exception ec) {
            std::cout << ec.what() << std::endl;
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }
    std::string make_move() {
        std::string move;
        std::cout << "Your turn:";
        std::cin >> move;
        return move;
    }
    static std::optional<SeabattleField::ShotResult> stringToShorRes(const std::string_view &str) {
        if (str.size() != 1) return std::nullopt;
        char r = str[0] - '0';
        if (r > 2 || r < 0) return std::nullopt;
        return static_cast<SeabattleField::ShotResult>(r);
    }
    //void how_initiative(SeabattleField::ShotResult sr, bool &init) {
    //    switch (sr) {
    //    case SeabattleField::ShotResult::HIT:
    //        std::cout << "Hit" << std::endl;
    //        break;
    //    case SeabattleField::ShotResult::KILL:
    //        std::cout << "Kill" << std::endl;
    //        break;
    //    case SeabattleField::ShotResult::MISS:
    //        std::cout << "Miss" << std::endl;
    //        init = !init;
    //        break;
    //    }
    //}
    // TODO: добавьте методы по вашему желанию

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);
    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Waiting for connectoin..."sv << std::endl;
    boost::system::error_code ec;
    tcp::socket socket(io_context);
    acceptor.accept(socket, ec);
    if (ec) {
        std::cout << "Can`t accept connectoin"sv << std::endl;
        return;
    }
    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);
    if (ec) {
        std::cout << "Wrong IP format"sv << std::endl;
        return;
    }
    SeabattleAgent agent(field);
    net::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(endpoint);
    agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
