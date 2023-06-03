#pragma once
#include "http_server.h"
#include "model.h"

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
using namespace std::literals;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

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
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
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
    StringResponse HandleRequest(StringRequest&& req);
    std::string GetMapBodyJson(std::string_view requestTarget, http::status& status);
    std::string StatusToJson(std::string_view code, std::string_view message);
    StringResponse MakeStringResponse(
        http::status status, std::string_view requestTarget, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APPLICATION_JSON);
    StringResponse MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APPLICATION_JSON);

};

}  // namespace http_handler
