#include <boost/asio.hpp>
#include "audio.h"
#include <iostream>
#include <string>
#include <string_view>

namespace net = boost::asio;
using net::ip::udp;
using namespace std::literals;

class Radio {
    static constexpr size_t fps = 44000;
    static constexpr size_t max_buffer_size = 4400;
    
public:
    bool server(char** argv) {
        Player player(ma_format_u8, 1);
        std::string str;
        std::cout << "Press Enter to playing message..." << std::endl;
        std::getline(std::cin, str);
        
        try {
            int port = std::stoi(std::string(argv[2]));
            std::cout << "Port: " << port << std::endl;
            boost::asio::io_context io_context;
            udp::socket socket(io_context, udp::endpoint(udp::v4(), port));
            size_t max_buffer_size_fr = max_buffer_size* player.GetFrameSize();
            std::unique_ptr<char[]> recv_buf = std::make_unique<char[]>(max_buffer_size_fr);
            udp::endpoint remote_endpoint;
            while (true) {
                // ѕолучаем не только данные, но и endpoint клиента
                auto size = socket.receive_from(net::buffer(recv_buf.get(), max_buffer_size_fr), remote_endpoint);
                player.PlayBuffer(recv_buf.get(), max_buffer_size_fr, 0.1s);
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        return false;
    }
    bool client(char** argv) {
        Recorder recorder(ma_format_u8, 1);
        std::string str;
        net::io_context io_context;
        try {
            int port = std::stoi(std::string(argv[3]));
            std::cout << "Port: " << port << std::endl;
            udp::socket socket(io_context, udp::v4()); 
            boost::system::error_code ec;
            auto endpoint = udp::endpoint(net::ip::make_address(argv[2], ec), port);

            std::cout << "Press Enter to record message..." << std::endl;
            std::getline(std::cin, str);
            Recorder::RecordingResult rec_result;
            
            while (true) {
                rec_result = recorder.Record(max_buffer_size, 0.1s);
                socket.send_to(net::buffer(rec_result.data.data(), rec_result.frames*recorder.GetFrameSize()), endpoint);
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

        return false;
    }
};

int main(int argc, char** argv) {
    Radio radio;
    if ((argc == 3) && "server"sv == std::string(argv[1])) {
        radio.server(argv);
    } else if ((argc == 4) && "client"sv == std::string(argv[1])) {
        radio.client(argv);
    } else {
        std::cout << "Usage: "sv << argv[0] << " server <port>"sv << std::endl;
        std::cout << "Usage: "sv << argv[0] << " client <server IP> <port>"sv << std::endl;
        //return 1;
    }
    Recorder recorder(ma_format_u8, 1);
    Player player(ma_format_u8, 1);

    while (true) {
        std::string str;

        std::cout << "Press Enter to record message..." << std::endl;
        std::getline(std::cin, str);

        auto rec_result = recorder.Record(65000, 1.5s);
        std::cout << "Recording done" << std::endl;

        player.PlayBuffer(rec_result.data.data(), rec_result.frames, 1.5s);
        std::cout << "Playing done" << std::endl;
    }

    return 0;
}
