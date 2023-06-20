#include "base.h"
#include <boost/beast/http.hpp>
#include "../app.h"

namespace http_handler {
std::unordered_map<std::string_view, std::string_view> ContentType::type{
    { FE::TEXT_HTML_1, TEXT_HTML },
    { FE::TEXT_HTML_2,   TEXT_HTML },
    { FE::TEXT_CSS,      TEXT_CSS },
    { FE::TEXT_PLAIN,    TEXT_PLAIN },
    { FE::TEXT_JS,       TEXT_JS },
    { FE::APP_JSON,      APP_JSON },
    { FE::APP_XML,       APP_XML },
    { FE::IMAGE_PNG,     IMAGE_PNG },
    { FE::IMAGE_JPG_1,   IMAGE_JPG },
    { FE::IMAGE_JPG_2,   IMAGE_JPG },
    { FE::IMAGE_JPG_3,   IMAGE_JPG },
    { FE::IMAGE_GIF,     IMAGE_GIF },
    { FE::IMAGE_BMP,     IMAGE_BMP },
    { FE::IMAGE_ICO,     IMAGE_ICO },
    { FE::IMAGE_TIFF_1,  IMAGE_TIFF },
    { FE::IMAGE_TIFF_2,  IMAGE_TIFF },
    { FE::IMAGE_SVG_1,   IMAGE_SVG },
    { FE::IMAGE_SVG_2,   IMAGE_SVG },
    { FE::AUDIO_MP3,     AUDIO_MP3 },
    { FE::EMPTY,         EMPTY },
};

// Создаёт StringResponse с заданными параметрами
StringResponse Base::MakeStringResponse(
        http::status status, std::string_view responseText, unsigned http_version,
        bool keep_alive, std::string_view content_type,
        bool no_cache) const {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (no_cache)
        response.set(http::field::cache_control, "no-cache"sv);
    response.body() = responseText;
    response.content_length(responseText.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse Base::MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type) const {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.set(http::field::allow, "GET, HEAD"sv);
    auto ans_v = "Invalid method"sv;
    response.body() = ans_v;
    response.content_length(ans_v.size());
    response.keep_alive(keep_alive);
    return response;
}

FileRequestResult Base::HandleRequest(StringRequest&& req) {
    // Format response
    try {
        switch (req.method()) {
        case http::verb::get:
            return MakeGetResponse(req, true);
        case http::verb::head:
            return MakeGetResponse(req, false);
        case http::verb::post:
            return MakePostResponse(req);
        default:
            return MakeBadResponse(http::status::method_not_allowed,
                req.version(), req.keep_alive());
        }
    }
    catch (std::exception ex) {
        return MakeStringResponse(http::status::bad_request,
            app::JsonMessage("badRequest", "Server error"s + ex.what()),
            req.version(), req.keep_alive());
    }
}



}