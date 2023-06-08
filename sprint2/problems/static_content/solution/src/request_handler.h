#pragma once
#include <boost/json.hpp>
#include <filesystem>
#include <iostream>

#include "http_server.h"
#include "model.h"

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace js = boost::json;
namespace fs = std::filesystem;
using namespace std::literals;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

enum class TypeRequest {
    None,
    StaticFiles,
    Maps,
    Map
};

struct ContentType {
    
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

class ModelToJson {
public:
    explicit ModelToJson(model::Game& game)
        : game_{game} {
    }
    std::string GetMaps();
    std::string GetMap(std::string nameMap);
private:
    model::Game& game_;
    static js::array GetRoads(const model::Map::Roads& roads);
    static js::array GetBuildings(const model::Map::Buildings& buildings);
    static js::array GetOffice(const model::Map::Offices& offices);
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, const fs::path &static_path)
        : game_{ game }, static_path_{ check_static_path(static_path)} {

    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        send(HandleRequest(std::forward<decltype(req)>(req)));
    }

private:
    model::Game& game_;
    const fs::path& static_path_;
    StringResponse HandleRequest(StringRequest&& req);
    std::string GetMapBodyJson(std::string_view requestTarget, http::status& status);
    std::string StatusToJson(std::string_view code, std::string_view message);
    StringResponse MakeStringResponse(
        http::status status, std::string_view requestTarget, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APPLICATION_JSON);
    StringResponse MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APPLICATION_JSON);

    static fs::path check_static_path(const fs::path& path_static);

};

}  // namespace http_handler
