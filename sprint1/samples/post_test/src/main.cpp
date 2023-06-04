// Подключаем заголовочный файл <sdkddkver.h> в системе Windows,
// чтобы избежать предупреждения о неизвестной версии Platform SDK,
// когда используем заголовочные файлы библиотеки Boost.Asio
#ifdef WIN32
#include <sdkddkver.h>
#endif
#include <boost/asio.hpp>
#include <iostream>
#include <syncstream>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
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

    net::io_context io{2};
    auto strand = net::make_strand(io);

    net::post(strand, [] { osync(std::cout) << "Cat, "; });

    net::post(strand, [] { osync(std::cout) << "Fish, "; });

    net::post(io, [] { osync(std::cout) << "Dog, "; });

    RunWorkers(2, [&io] {
        io.run();
        });

    std::cout << std::endl;
}

// ---4---
//int main() {
//    net::io_context io;
//
//    net::post(io, [&io] {
//        std::cout << "Read book" << std::endl;
//
//        net::post(io, [] { std::cout << "Write essay" << std::endl; });
//        });
//
//    net::post(io, [] { std::cout << "Play football" << std::endl; });
//
//    std::cout << "Watch movie" << std::endl;
//    io.run();
//    std::cout << "Study English" << std::endl;
//}

// ---3---
//int main() {
//    net::io_context io{2};
//    auto strand = net::make_strand(io);
//    int value = 10;
//
//    net::post(strand, [&value, &strand] {  // Эта функция будет первой внутри strand
//        value += 20;                       // (1)
//
//        net::post(strand, [&value] {  // Этот post поставит функцию третьей по счёту
//            value += 10;                   // (3)
//            });
//        });
//
//    net::post(strand, [&value, &strand] {  // Эта функция будет второй внутри strand
//        value *= 40;                       // (2)
//
//        net::post(strand, [&value] {  // Этот post поставит функцию четвёртой по счёту
//            value *= 20;                   // (4)
//            });
//        });
//
//    RunWorkers(2, [&io] {
//        io.run();
//        });
//
//    assert(value == ((10 + 20) * 40 + 10) * 20);
//}


// ---2---
// template <typename Fn>
//void RunWorkers(unsigned n, const Fn& fn) {
//    n = std::max(1u, n);
//    std::vector<std::jthread> workers;
//    workers.reserve(n - 1);
//    while (--n) {
//        workers.emplace_back(fn);
//    }
//    fn();
//}
//int main() {
//    using osync = std::osyncstream;
//
//    net::io_context io;
//    std::cout << "Eat. Thread id: " << std::this_thread::get_id() << std::endl;
//
//    net::post(io, [] {  // Моем посуду
//        osync(std::cout) << "Wash dishes. Thread id: " << std::this_thread::get_id() << std::endl;
//        });
//
//    net::post(io, [] {  // Прибираемся на столе
//        osync(std::cout) << "Cleanup table. Thread id: " << std::this_thread::get_id() << std::endl;
//        });
//
//    net::post(io, [&io] {  // Пылесосим комнату
//        osync(std::cout) << "Vacuum-clean room. Thread id: " << std::this_thread::get_id()
//            << std::endl;
//
//        net::post(io, [] {  // После того, как пропылесосили, асинхронно моем пол
//            osync(std::cout) << "Wash floor. Thread id: " << std::this_thread::get_id()
//                << std::endl;
//            });
//
//        net::post(io, [] {  // Асинхронно опустошаем пылесборник пылесоса
//            osync(std::cout) << "Empty vacuum cleaner. Thread id: " << std::this_thread::get_id()
//                << std::endl;
//            });
//        });
//
//    std::cout << "Work. Thread id: " << std::this_thread::get_id() << std::endl;
//
//    RunWorkers(2, [&io] {  // Асинхронные операции выполняются двумя потоками
//        io.run();
//        });
//    std::cout << "Sleep" << std::endl;
//}


// ---1---
//#include <boost/asio/ip/tcp.hpp>
//#include <boost/asio/write.hpp>
//#include <iostream>
//
//namespace net = boost::asio;
//using tcp = net::ip::tcp;
//using namespace std::literals;
//
//int main() {
//    net::io_context io;
//    std::cout << "Eat. Thread id: " << std::this_thread::get_id() << std::endl;
//
//    net::post(io, [] {
//        std::cout << "Wash dishes. Thread id: " << std::this_thread::get_id() << std::endl;
//        });
//
//    net::post(io, [] {
//        std::cout << "Cleanup table. Thread id: " << std::this_thread::get_id() << std::endl;
//        });
//
//    std::cout << "Work" << std::endl;
//    std::jthread{[&io] {
//        io.run();
//    }}.join();
//    std::cout << "Sleep" << std::endl;
//
//    //net::io_context io;
//    //std::cout << "eat. thread id: " << std::this_thread::get_id() << std::endl;
//
//    //net::post(io, [] {
//    //    std::cout << "wash dishes. thread id: " << std::this_thread::get_id() << std::endl;
//    //    });
//
//    //net::post(io, [] {
//    //    std::cout << "cleanup table. thread id: " << std::this_thread::get_id() << std::endl;
//    //    });
//
//    //std::cout << "work" << std::endl;
//    //io.run();
//    //std::cout << "sleep" << std::endl;
//    return 0;
//} 
//
// 
