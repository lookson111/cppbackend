#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <filesystem>
#include <iostream>
#include <variant>

#include "http_server.h"
#include "model.h"
#include "log.h"
#include "app.h"

namespace http_handler {
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;
namespace sys = boost::system;
using namespace std::literals;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;
using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;

enum class TypeRequest {
    None,
    StaticFiles,
    Maps,
    Map,
    Join,
    Players
};

struct FileExtension {
    // file extension
    constexpr static std::string_view TEXT_HTML_1 = ".htm"sv;
    constexpr static std::string_view TEXT_HTML_2 = ".html"sv;
    constexpr static std::string_view TEXT_CSS = ".css"sv;
    constexpr static std::string_view TEXT_PLAIN = ".txt"sv;
    constexpr static std::string_view TEXT_JS = ".js"sv;
    constexpr static std::string_view APP_JSON = ".json"sv;
    constexpr static std::string_view APP_XML = ".xml"sv;
    constexpr static std::string_view IMAGE_PNG = ".png"sv;
    constexpr static std::string_view IMAGE_JPG_1 = ".jpg"sv;
    constexpr static std::string_view IMAGE_JPG_2 = ".jpe"sv;
    constexpr static std::string_view IMAGE_JPG_3 = ".jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = ".gif"sv;
    constexpr static std::string_view IMAGE_BMP = ".bmp"sv;
    constexpr static std::string_view IMAGE_ICO = ".ico"sv;
    constexpr static std::string_view IMAGE_TIFF_1 = ".tiff"sv;
    constexpr static std::string_view IMAGE_TIFF_2 = ".tif"sv;
    constexpr static std::string_view IMAGE_SVG_1 = ".svg"sv;
    constexpr static std::string_view IMAGE_SVG_2 = ".svgz"sv;
    constexpr static std::string_view AUDIO_MP3 = ".mp3"sv;
    constexpr static std::string_view EMPTY = ""sv;
};
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML  = "text/html"sv; // .htm, .html
    constexpr static std::string_view TEXT_CSS   = "text/css"sv; // .css
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv; // .txt
    constexpr static std::string_view TEXT_JS    = "text/javascript"sv; // .js
    constexpr static std::string_view APP_JSON   = "application/json"sv; // .json
    constexpr static std::string_view APP_XML    = "application/xml"sv; // .xml
    constexpr static std::string_view IMAGE_PNG  = "image/png"sv; // .png
    constexpr static std::string_view IMAGE_JPG  = "image/jpeg"sv; // .jpg, .jpe, .jpeg
    constexpr static std::string_view IMAGE_GIF  = "image/gif"sv; // .gif
    constexpr static std::string_view IMAGE_BMP  = "image/bmp"sv; // .bmp
    constexpr static std::string_view IMAGE_ICO  = "image/vnd.microsoft.icon"sv; // .ico
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv; // .tiff, .tif
    constexpr static std::string_view IMAGE_SVG  = "image/svg+xml"sv; // .svg, .svgz
    constexpr static std::string_view AUDIO_MP3  = "audio/mpeg"sv; // .mp3
    constexpr static std::string_view EMPTY      = "application/octet-stream"sv;

    static std::string_view get(std::string_view key) {
        if (!type.contains(key))
            return EMPTY;
        return type.at(key);
    }
private:
    typedef FileExtension FE;
    static std::unordered_map<std::string_view, std::string_view> type;
};

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    RequestHandler(const fs::path& static_path, Strand api_strand, model::Game& game)
        : static_path_{ CheckStaticPath(static_path) }
        , api_strand_{ api_strand }
        , app_(game) {
        }


    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;


    template <typename Body, typename Allocator, typename Send>
    void operator()(tcp::endpoint, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        auto version = req.version();
        auto keep_alive = req.keep_alive();
        std::string_view target = StringRequest(req).target();
        std::string_view api = "/api/"sv;
        bool is_target = target.size() > api.size() && (target.substr(0, api.size()-1) == api);
        try {
            /*req относится к API?*/
            if (is_target) {
                auto handle = [self = shared_from_this(), send,
                    req = std::forward<decltype(req)>(req), version, keep_alive] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_strand_.running_in_this_thread());
                        return send(self->HandleApiRequest(req));
                    }
                    catch (...) {
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return net::dispatch(api_strand_, handle);
            }
            // Возвращаем результат обработки запроса к файлу
            return std::visit(
                [&send](auto&& result) {
                    send(std::forward<decltype(result)>(result));
                },
                HandleFileRequest(req));
        }
        catch (...) {
            send(ReportServerError(version, keep_alive));
        }
    }


    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        auto res = HandleRequest(std::forward<decltype(req)>(req));
        if (std::holds_alternative<StringResponse>(res))
            send(std::get<StringResponse>(res));
        else
            send(std::get<FileResponse>(res));
    }

private:
    //using VariantResponse = std::variant<StringResponse, FileResponse>;

    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    StringResponse HandleApiRequest(const StringRequest& req) const;
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    app::App app_;
    const fs::path static_path_;
    Strand api_strand_;

    FileRequestResult HandleRequest(StringRequest&& req);
    StringResponse MakeStringResponse(
        http::status status, std::string_view requestTarget, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APP_JSON,
        bool no_cache = false) const;
    StringResponse MakeBadResponse(
        http::status status, unsigned http_version,
        bool keep_alive, std::string_view content_type = ContentType::APP_JSON) const;
    FileRequestResult MakeGetResponse(const StringRequest& req, bool with_body) const;
    FileRequestResult MakePostResponse(const StringRequest& req);
    FileRequestResult StaticFilesResponse(
        std::string_view responseText, bool with_body,
        unsigned http_version, bool keep_alive);
    static fs::path CheckStaticPath(const fs::path& path_static);
    bool CheckFileExist(std::string& file) const;
    bool FileInRootStaticDir(std::string_view file) const;
};

}  // namespace http_handler
