#include <iostream>
#include <pqxx/pqxx>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <mutex>


class ConnectionPool {
    using PoolType = ConnectionPool;
    using ConnectionPtr = std::shared_ptr<pqxx::connection>;

public:
    class ConnectionWrapper {
    public:
        ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept
            : conn_{std::move(conn)}
            , pool_{&pool} {
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
/*
int main() {
    using namespace std::chrono;
    try {
        const char* db_url = std::getenv("DB_URL");
        if (!db_url) {
            throw std::runtime_error("DB URL is not specified");
        }
        const auto start_time = steady_clock::now();
        ConnectionPool conn_pool{10, [db_url] {
                                     auto conn = std::make_shared<pqxx::connection>(db_url);
                                     conn->prepare("select_one", "SELECT 1;");
                                     return conn;
                                 }};
        auto pool_creation_time = steady_clock::now();
        steady_clock::time_point conn_time;
        steady_clock::time_point tx_construction_time;
        steady_clock::time_point query_end_time;
        steady_clock::time_point tx_end_time;
        steady_clock::time_point conn_end_time;
        {
            auto conn = conn_pool.GetConnection();
            conn_time = steady_clock::now();
            {
                pqxx::read_transaction tx{*conn};
                tx_construction_time = steady_clock::now();
                std::ignore = tx.exec_prepared1("select_one").as<int>();
                query_end_time = steady_clock::now();
            }
            tx_end_time = steady_clock::now();
        }
        conn_end_time = steady_clock::now();

        std::cout << "Pool creation time: "
                  << duration_cast<duration<double> >(pool_creation_time - start_time).count()
                  << std::endl
                  << "-------------" << std::endl;
        std::cout << "Getting connection time: "
                  << duration_cast<duration<double> >(conn_time - pool_creation_time).count()
                  << std::endl;
        std::cout << "Create transaction time: "
                  << duration_cast<duration<double> >(tx_construction_time - conn_time).count()
                  << std::endl;
        std::cout << "Query time: "
                  << duration_cast<duration<double> >(query_end_time - tx_construction_time).count()
                  << std::endl;
        std::cout << "Destroy transaction time: "
                  << duration_cast<duration<double> >(tx_end_time - query_end_time).count()
                  << std::endl;
        std::cout << "Close connection time: "
                  << duration_cast<duration<double> >(conn_end_time - tx_end_time).count()
                  << std::endl;
        std::cout << "Total time: "
                  << duration_cast<duration<double> >(conn_end_time - pool_creation_time).count()
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
} */

int main() {
    const char* db_url = std::getenv("DB_URL");
    if (!db_url) {
        return 1;
    }
    ConnectionPool conn_pool{1, [db_url] {
                                 return std::make_shared<pqxx::connection>(db_url);
                             }};
    auto conn1 = conn_pool.GetConnection();
    auto conn2 = conn_pool.GetConnection();
}



