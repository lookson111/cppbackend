#include "request_handler.h"

namespace http_handler {
//using namespace std::literals;


// Создаёт StringResponse с заданными параметрами
StringResponse RequestHandler::MakeStringResponse(
        http::status status, std::string_view body, unsigned http_version,
        bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse RequestHandler::MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::allow, "GET, HEAD"sv);
    auto ans_v = "Invalid method"sv;
    response.body() = ans_v;
    response.content_length(ans_v.size());
    response.keep_alive(keep_alive);
    return response;
}
StringResponse RequestHandler::HandleRequest(StringRequest&& req) {
    // Подставьте сюда код из синхронной версии HTTP-сервера
    //return MakeStringResponse(http::status::ok, "OK"sv, req.version(), req.keep_alive());
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
    const auto text_bad_response = [&](http::status status) {
        return MakeBadResponse(status, req.version(), req.keep_alive());
    };
    // Format response
    switch (req.method()) {
    case http::verb::get:
        return text_response(http::status::ok, "Hello, "s + 
            std::string{req.target().substr(1)});
    case http::verb::head:
        return text_response(http::status::ok, "");
    default:
        return text_bad_response(http::status::method_not_allowed);
    }
}
}  // namespace http_handler
