#include <iostream>
#include <pqxx/pqxx>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


class ConnectionPool {
    using PoolType = ConnectionPool;
    using ConnectionPtr = std::shared_ptr<pqxx::connection>;

public:
    class ConnectionWrapper {
    public:
        ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept
            : conn_{std::move(conn)}
            , pool_{&pool} {
            // Schedule the timer for the first time:
            timer.async_wait(tick);
            // Enter IO loop. The timer will fire for the first time 1 second from now:
            io_service.run();
        }

        ConnectionWrapper(const ConnectionWrapper&) = delete;
        ConnectionWrapper& operator=(const ConnectionWrapper&) = delete;

        ConnectionWrapper(ConnectionWrapper&&) = default;
        ConnectionWrapper& operator=(ConnectionWrapper&&) = default;

        pqxx::connection& operator*() const& noexcept {
            return *conn_;
        }
        pqxx::connection& operator*() const&& = delete;

        pqxx::connection* operator->() const& noexcept {
            return conn_.get();
        }

        ~ConnectionWrapper() {
            if (conn_) {
                pool_->ReturnConnection(std::move(conn_));
            }
        }

    private:
        std::shared_ptr<pqxx::connection> conn_;
        PoolType* pool_;
        boost::asio::io_service io_service;
        boost::posix_time::seconds interval{1};
        boost::asio::deadline_timer timer{io_service, interval};
        static void tick(const boost::system::error_code& /*e*/) {
            std::cout << "tick" << std::endl;
            throw std::logic_error("");
        }
    };

    // ConnectionFactory is a functional object returning std::shared_ptr<pqxx::connection>
    template <typename ConnectionFactory>
    ConnectionPool(size_t capacity, ConnectionFactory&& connection_factory) {
        pool_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            pool_.emplace_back(connection_factory());
        }
    }

    ConnectionWrapper GetConnection() {
        std::unique_lock lock{mutex_};
        // Блокируем текущий поток и ждём, пока cond_var_ не получит уведомление и не освободится
        // хотя бы одно соединение
        cond_var_.wait(lock, [this] {
            return used_connections_ < pool_.size();
        });
        // После выхода из цикла ожидания мьютекс остаётся захваченным
        return {std::move(pool_[used_connections_++]), *this};
    }

private:
    void ReturnConnection(ConnectionPtr&& conn) {
        // Возвращаем соединение обратно в пул
        {
            std::lock_guard lock{mutex_};
            assert(used_connections_ != 0);
            pool_[--used_connections_] = std::move(conn);
        }
        // Уведомляем один из ожидающих потоков об изменении состояния пула
        cond_var_.notify_one();
    }

    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<ConnectionPtr> pool_;
    size_t used_connections_ = 0;
};

int main() {
    const char* db_url = std::getenv("DB_URL");
    if (!db_url) {
        return 1;
    }
    try {
        ConnectionPool conn_pool{1, [db_url] {
                                    return std::make_shared<pqxx::connection>(db_url);
                                }};
        auto conn1 = conn_pool.GetConnection();
        auto conn2 = conn_pool.GetConnection();
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    try {
        ConnectionPool conn_pool{1, [db_url] {
                                    return std::make_shared<pqxx::connection>(db_url);
                                }};
        auto conn1 = conn_pool.GetConnection();
        auto conn2 = conn_pool.GetConnection();
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    try {
        ConnectionPool conn_pool{1, [db_url] {
                                    return std::make_shared<pqxx::connection>(db_url);
                                }};
        auto conn3 = conn_pool.GetConnection();
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    try {
        ConnectionPool conn_pool{1, [db_url] {
                                    return std::make_shared<pqxx::connection>(db_url);
                                }};
        auto conn4 = conn_pool.GetConnection();
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}



