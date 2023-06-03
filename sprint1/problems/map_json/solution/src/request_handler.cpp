#include "request_handler.h"
//	Основные	заголовочные	файлы	для	Boost.Spirit.
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
//	Мы	будем	использовать	функцию	bind()	из	Boost.Spirit,	потому	что	она	лучше
//	взаимодействует	с	парсерами.
#include <boost/spirit/include/phoenix_bind.hpp>

namespace http_handler {
using boost::spirit::qi;
void parse_target() {
     qi::rule<const char*, void()> timezone_parser
	    =	-(	//	унарный	минус	значит	“опциональное”	правило
		        //	Нулевое	смещение
				char_('Z')[	bind(
                    &datetime::set_zone_offset, &ret, datetime::OFFSET_Z
                    ) ]
                |	//	ИЛИ
				//	Смещение	задано	цифрами
				((char_('+')|char_('-'))	>>	u2_	>>	':'	>>	u2_)	[
                    bind(&set_zone_offset, ref(ret), _1, _2, _3)
                ]
        );
}

std::string ModelToJson::GetMaps() {
    std::string mapsStr;
    const auto &maps = game_.GetMaps();

    return mapsStr;
}
std::string ModelToJson::GetMap(std::string_view nameMap) {
    std::string mapStr;
    model::Map::Id idmap{nameMap};
    const auto &map = game_.FindMap({idmap});

    return mapStr;
}

// Создаёт StringResponse с заданными параметрами
StringResponse RequestHandler::MakeStringResponse(
        http::status status, std::string_view requestTarget, unsigned http_version,
        bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    ModelToJson jmodel(game_);
    constexpr auto svTarget = "/api/v1/maps"sv;
/*
    response.set(http::field::content_type, content_type);
    if (requestTarget.find(svTarget) == requestTarget.npos) {
        // TODO bad respose
    }
    std::string_view mapStr = requestTarget.substr(svTarget.size(), 
        requestTarget.size() - svTarget.size());
    
    if (mapStr == ""sv) {
        response.body() = jmodel.GetMaps();
    } else {
        std::string_view mapName = mapStr.substr(reqStr);
        response.body() = jmodel.GetMap();
    }
*/
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
