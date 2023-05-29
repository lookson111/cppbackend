// Подключаем заголовочный файл <sdkddkver.h> в системе Windows,
// чтобы избежать предупреждения о неизвестной версии Platform SDK,
// когда используем заголовочные файлы библиотеки Boost.Asio
#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <syncstream>
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;

// Run func fn in n thread, with this
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

int main() {
    using osync = std::osyncstream;

    net::io_context io;
    auto strand1 = net::make_strand(io);

    net::post(strand1, [strand1] {  // (1)
        net::post(strand1, [] {     // (2)
            osync(std::cout) << 'A';
        });
        net::dispatch(strand1, [] {  // (3)
            osync(std::cout) << 'B';
        });
        osync(std::cout) << 'C';
    });

    auto strand2 = net::make_strand(io);
    // Эти функции выполняются в strand2
    net::post(strand2, [strand2] {  // (4)
        net::post(strand2, [] {     // (5)
            osync(std::cout) << 'D';
        });
        net::dispatch(strand2, [] {  // (6)
            osync(std::cout) << 'E';
        });
        osync(std::cout) << 'F';
    });

    RunWorkers(2, [&io] {
        io.run();
    });
}
// ---3---
/*
int main() {
    using osync = std::osyncstream;

    net::io_context io;
    auto strand = net::make_strand(io);

    net::post(strand, [strand] {  // (1)
        net::post(strand, [] {    // (2)
            osync(std::cout) << 'A';
        });
        net::dispatch(strand, [] {  // (3)
            osync(std::cout) << 'B';
        });
        osync(std::cout) << 'C';
    });

    // Теперь тут используется strand, а не io
    net::post(strand, [strand] {  // (4)
        net::post(strand, [] {    // (5)
            osync(std::cout) << 'D';
        });
        net::dispatch(strand, [] {  // (6)
            osync(std::cout) << 'E';
        });
        osync(std::cout) << 'F';
    });

    RunWorkers(2, [&io] {
        io.run();
    });
} */
// ---2---
/*
int main() {
    using osync = std::osyncstream;

    net::io_context io;
    auto strand = net::make_strand(io);

    net::post(strand, [strand] {
        net::post(strand, [] {
            osync(std::cout) << 'A';
        });
        net::dispatch(strand, [] {
            osync(std::cout) << 'B';
        });
        osync(std::cout) << 'C';
    });

    net::post(io, [&io] {
        net::post(io, [] {
            osync(std::cout) << 'D';
        });
        net::dispatch(io, [] {
            osync(std::cout) << 'E';
        });
        osync(std::cout) << 'F';
    });

    RunWorkers(2, [&io] {
        io.run();
    });
} */
// --- 1 ----
/*
int main() {
    net::io_context io;

    net::post(io, [&io] {  // (1)
        std::cout << 'A';
        net::post(io, [] {  // (2)
            std::cout << 'B';
        });
        std::cout << 'C';
    });

    net::dispatch(io, [&io] {  // (3)
        std::cout << 'D';
        net::post(io, [] {  // (4)
            std::cout << 'E';
        });
        net::defer(io, [] {  // (5)
            std::cout << 'F';
        });
        net::dispatch(io, [] {  // (6)
            std::cout << 'G';
        });
        std::cout << 'H';
    });

    std::cout << 'I';
    io.run();
    std::cout << 'J';
    return 0;
} 
*/
 
