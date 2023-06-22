#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <filesystem>
#include <iostream>
#include <variant>

#include "../http_server.h"
#include "../model.h"
#include "../log.h"
#include "../app.h"

#include "file.h"
#include "api.h"

namespace http_handler {

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    RequestHandler(const fs::path& static_path, Strand api_strand, model::Game& game)
        : file{ static_path }
        , api_strand_{ api_strand }
        , api(game) {
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
                        return send(std::get<StringResponse>(
                            self->api.HandleRequest(req)));
                        /*return std::visit(
                            [&send](auto&& result) {
                                send(std::forward<decltype(result)>(result));
                            },
                            file.HandleRequest(req)
                        );*/
                            //std::forward<decltype(req)>(req)));
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
                file.HandleRequest(req));
                    //std::forward<decltype(req)>(req)));
        }
        catch (...) {
            //send(ReportServerError(version, keep_alive));
        }
    }


    //template <typename Body, typename Allocator, typename Send>
    //void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    //    // Обработать запрос request и отправить ответ, используя send
    //    auto res = HandleRequest(std::forward<decltype(req)>(req));
    //    if (std::holds_alternative<StringResponse>(res))
    //        send(std::get<StringResponse>(res));
    //    else
    //        send(std::get<FileResponse>(res));
    //}

private:
    //using VariantResponse = std::variant<StringResponse, FileResponse>;

    //FileRequestResult HandleFileRequest(const StringRequest& req) const;
    //StringResponse HandleApiRequest(const StringRequest& req) const;
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    Api api;
    File file;
    //app::App app_;
    //const fs::path static_path_;
    Strand api_strand_;

    //FileRequestResult HandleRequest(StringRequest&& req);
    //StringResponse MakeStringResponse(
    //    http::status status, std::string_view requestTarget, unsigned http_version,
    //    bool keep_alive, std::string_view content_type = ContentType::APP_JSON,
    //    bool no_cache = false) const;
    //StringResponse MakeBadResponse(
    //    http::status status, unsigned http_version,
    //    bool keep_alive, std::string_view content_type = ContentType::APP_JSON) const;
    //FileRequestResult MakeGetResponse(const StringRequest& req, bool with_body) const;
    //FileRequestResult MakePostResponse(const StringRequest& req);
    //FileRequestResult StaticFilesResponse(
    //    std::string_view responseText, bool with_body,
    //    unsigned http_version, bool keep_alive);
    //static fs::path CheckStaticPath(const fs::path& path_static);
    //bool CheckFileExist(std::string& file) const;
    //bool FileInRootStaticDir(std::string_view file) const;
};

}  // namespace http_handler
