#pragma once
#include "base.h"
#include "../app.h"

namespace http_handler {

class Api : public Base {
private:
	app::App app_;
	FileRequestResult MakeGetResponse(const StringRequest& req, bool with_body) const;
	FileRequestResult MakePostResponse(const StringRequest& req);
    TypeRequest parse_target(std::string_view target, std::string& res) const;

public:
	Api(model::Game& game)
		: app_(game) {}


};
} // namespace http_handler
