#include "file.h"
#include <boost/algorithm/string.hpp>
#include "../app.h"

namespace http_handler {


TypeRequest parse_target(std::string_view target, std::string& res) {
    std::string_view api = "/api/"sv;
    std::string_view maps = "/api/v1/maps"sv;
    std::string_view join = "/api/v1/game/join"sv;
    std::string_view players = "/api/v1/game/players"sv;
    res = "";
    std::string uriDecode = http_server::uriDecode(target);
    // request stitic files
    auto pos = target.find(api);
    if (pos == target.npos) {
        res = target;
        return TypeRequest::StaticFiles;
    }
    // finded maps
    pos = target.find(maps);
    if (pos != target.npos) {
        if (target.size() == maps.size())
            return TypeRequest::Maps;
        res = target.substr(maps.size() + 1, target.size() - maps.size());
        return TypeRequest::Map;
    }
    // join game
    pos = target.find(join);
    if (pos != target.npos) {
        return TypeRequest::Join;
    }
    if (target.find(players) != target.npos) {
        return TypeRequest::Players;
    }
    return TypeRequest::None;
}

fs::path File::CheckStaticPath(const fs::path& path_static) {
    auto path = fs::weakly_canonical(path_static);
    if (!fs::is_directory(path)) {
        std::string msgError = "Static path "s + path.generic_string() +
            " is not exist";
        throw std::invalid_argument(msgError);
    }
    return path;
}

bool File::CheckFileExist(std::string& file) const {
    std::string fileName = fs::path(file).filename().string();
    auto path = fs::path(file).parent_path();
    for (const auto& entry : fs::directory_iterator{ path }) {
        std::string fn = entry.path().filename().string();
        if (boost::iequals(fn, fileName)) {
            file = entry.path().string();
            return true;
        }
    }
    //    if (fs::exists(filePath))
    //        return true;
    return false;
}
bool File::FileInRootStaticDir(std::string_view file) const {
    auto path = fs::weakly_canonical(file).string();
    if (path.find(static_path_.string()) == 0)
        return true;
    return false;
}

FileRequestResult File::StaticFilesResponse(
    std::string_view target, bool with_body,
    unsigned http_version, bool keep_alive) {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, http_version,
            keep_alive, ContentType::TEXT_PLAIN);
    };
    if (target == "/")
        target = "/index.html";
    std::string fullName = static_path_.string() + target.data();
    if (!FileInRootStaticDir(fullName))
        return text_response(http::status::bad_request, "Bad Request");
    if (!CheckFileExist(fullName))
        return text_response(http::status::not_found, "File not found");
    http::file_body::value_type file;
    if (sys::error_code ec; file.open(fullName.c_str(), beast::file_mode::read, ec), ec) {
        return text_response(http::status::gone,
            app::JsonMessage("Gone"sv, "Failed to open file "s + fullName));
    }
    http::response<http::file_body> res;
    res.version(http_version);
    res.result(http::status::ok);
    std::string ext = fs::path(fullName).extension().string();
    res.set(http::field::content_type, ContentType::get(ext).data());

    if (with_body)
        res.body() = std::move(file);
    // ����� prepare_payload ��������� ��������� Content-Length � Transfer-Encoding
    // � ����������� �� ������� ���� ���������
    res.prepare_payload();
    return std::move(res);
}


FileRequestResult File::MakeGetResponse(const StringRequest& req, bool with_body) const {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };
    std::string target;
    // if bad URI
    switch (parse_target(req.target(), target)) {
    case TypeRequest::StaticFiles:
        return StaticFilesResponse(target, with_body, req.version(),
            req.keep_alive());
    default:
        return text_response(http::status::bad_request,
            with_body ? app::JsonMessage("badRequest", "Bad request") : ""s
        );
    }
}

FileRequestResult File::MakePostResponse(const StringRequest& req) {
    const auto text_response = [&](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive(),
            ContentType::APP_JSON, true);
    };
    std::string target;
    // if bad URI
    switch (parse_target(req.target(), target)) {
    case TypeRequest::Join: {
        auto [body, err] = app_.ResponseJoin(req.body());
        http::status stat;
        switch (err) {
        case app::JoinError::BadJson:
        case app::JoinError::InvalidName:
            stat = http::status::bad_request;
            break;
        case app::JoinError::MapNotFound:
            stat = http::status::not_found;
            break;
        default:
            stat = http::status::ok;
        }
        return text_response(stat, body);

    }
    case TypeRequest::Players: {
        auto text = app::JsonMessage("invalidMethod"sv, "Invalid method"sv);
        auto resp = MakeStringResponse(http::status::method_not_allowed, text,
            req.version(), req.keep_alive(), ContentType::APP_JSON, true);
        resp.set(http::field::allow, "GET, HEAD"sv);
        return resp;
    }
    default:
        return text_response(http::status::bad_request,
            app::JsonMessage("badRequest", "Bad request"));
    }
}
}