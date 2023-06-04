#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>
#include <chrono>
#include <syncstream>
#include <memory>
#include <thread>
#include <atomic>
#include "hamburger.h"
namespace net = boost::asio;
namespace sys = boost::system;
using namespace std::chrono;
using Timer = net::steady_timer;
// �������, ������� ����� ������� �� ��������� ��������� ������
using OrderHandler = std::function<void(sys::error_code ec, int id, Hamburger* hamburger)>;
namespace ph = std::placeholders;

#include <atomic>

class ThreadChecker {
public:
    explicit ThreadChecker(std::atomic_int& counter)
        : counter_{ counter } {
    }

    ThreadChecker(const ThreadChecker&) = delete;
    ThreadChecker& operator=(const ThreadChecker&) = delete;

    ~ThreadChecker() {
        // assert ���������, ���� ����� ������� ������������ � �����������
        // �������� expected_counter_ ���������
        assert(expected_counter_ == counter_);
    }

private:
    std::atomic_int& counter_;
    int expected_counter_ = ++counter_;
};

class Logger {
public:
    explicit Logger(std::string id)
        : id_(std::move(id)) {
    }

    void LogMessage(std::string_view message) const {
        std::osyncstream os{std::cout};
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
           << "s] "sv << message << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
}; 

class Order : public std::enable_shared_from_this<Order> {
public:
    Order(net::io_context& io, int id, bool with_onion, OrderHandler handler)
        : io_{ io }
        , id_{ id }
        , with_onion_{ with_onion }
        , handler_{ std::move(handler) } {
    }

    // ��������� ����������� ���������� ������
    void Execute() {
        logger_.LogMessage("Order has been started."sv);
        RoastCutlet();
        if (with_onion_) {
            MarinadeOnion();
        }
    }
private:
    net::io_context& io_;
    int id_;
    bool with_onion_;
    bool delivered_ = false;
    bool onion_marinaded_ = false;
    OrderHandler handler_;
    Hamburger hamburger_;
    Timer roast_timer_{ io_, 1s };
    Timer marinade_timer_{ io_, 2s };
    Logger logger_{ std::to_string(id_) };
    std::atomic_int counter_{0};
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};

    void RoastCutlet() {
        logger_.LogMessage("Start roasting cutlet"sv);
        roast_timer_.async_wait(
            // OnRoasted ����� ������ ���������������� ������������ strand_
            net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
                self->OnRoasted(ec);
            }));
    }
    void OnRoasted(sys::error_code ec) {
        ThreadChecker checker{ counter_ };
        if (ec) {
            logger_.LogMessage("Roast error : "s + ec.what());
        }
        else {
            logger_.LogMessage("Cutlet has been roasted."sv);
            hamburger_.SetCutletRoasted();
        }
        CheckReadiness(ec);
    }
    void MarinadeOnion() {
        logger_.LogMessage("Start marinading onion"sv);
        marinade_timer_.async_wait(
            // OnOnionMarinaded ����� ������ ���������������� ������������ strand_
            net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
                self->OnOnionMarinaded(ec);
        }));
    }

    void OnOnionMarinaded(sys::error_code ec) {
        ThreadChecker checker{ counter_ };
        if (ec) {
            logger_.LogMessage("Marinade onion error: "s + ec.what());
        }
        else {
            logger_.LogMessage("Onion has been marinaded."sv);
            onion_marinaded_ = true;
        }
        CheckReadiness(ec);
    }

    void CheckReadiness(sys::error_code ec) {
        if (delivered_) {
            // �������, ���� ����� ��� ��������� ���� ������� ��������� �� ������
            return;
        }
        if (ec) {
            // � ������ ������ ���������� ������� � ������������� ��������� �����
            return Deliver(ec);
        }

        // ����� ����� �������� ���
        if (CanAddOnion()) {
            logger_.LogMessage("Add onion"sv);
            hamburger_.AddOnion();
        }

        // ���� ��� ���������� ���������� ������, ����������� ���
        if (IsReadyToPack()) {
            Pack();
        }
    }
    void Deliver(sys::error_code ec) {
        // ������ ������ �� ��������� ��������
        delivered_ = true;
        // ���������� ��������� � ������ ������ ���� nullptr, ���� �������� ������
        handler_(ec, id_, ec ? nullptr : &hamburger_);
    }
    [[nodiscard]] bool CanAddOnion() const {
        // ��� ����� ��������, ���� ������� ��������, ��� �����������, �� ���� �� ��������
        return hamburger_.IsCutletRoasted() && onion_marinaded_ && !hamburger_.HasOnion();
    }
    [[nodiscard]] bool IsReadyToPack() const {
        // ���� ������� �������� � ��� ��������, ��� �������, ��������� ����� �����������
        return hamburger_.IsCutletRoasted() && (!with_onion_ || hamburger_.HasOnion());
    }

    void Pack() {
        logger_.LogMessage("Packing"sv);

        // ������ ���������� ������� ���������� � ������� 0,5 �.
        auto start = steady_clock::now();
        while (steady_clock::now() - start < 500ms) {
        }

        hamburger_.Pack();
        logger_.LogMessage("Packed"sv);

        Deliver({});
    }
};

class Restaurant {
public:
    explicit Restaurant(net::io_context& io)
        : io_(io) {
    }

    int MakeHamburger(bool with_onion, OrderHandler handler) {
        const int order_id = ++next_order_id_;
        std::make_shared<Order>(io_, order_id, with_onion, std::move(handler))->Execute();
        return order_id;
    }

private:
    net::io_context& io_;
    int next_order_id_ = 0;
};

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

    const unsigned num_workers = 4;
    net::io_context io(num_workers);

    Restaurant restaurant{ io };
    Logger logger{ "main"s };
    auto print_result = [&logger](sys::error_code ec, int order_id, Hamburger* hamburger) {
        std::ostringstream os;
        if (ec) {
            os << "Order "sv << order_id << "failed: "sv << ec.what();
            return;
        }
        os << "Order "sv << order_id << " is ready. "sv << *hamburger;
        logger.LogMessage(os.str());
    };

    for (int i = 0; i < 16; ++i) {
        restaurant.MakeHamburger(i % 2 == 0, print_result);
    }
    RunWorkers(num_workers, [&io]{
        io.run();
    });
}
