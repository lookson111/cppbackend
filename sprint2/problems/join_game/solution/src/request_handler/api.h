#pragma once
#include "base.h"
#include "../app.h"

namespace http_handler {

class Api : Base {
private:
	app::App app_;
	FileRequestResult MakeGetResponse(const StringRequest& req, bool with_body) const;
	FileRequestResult MakePostResponse(const StringRequest& req);
public:
	Api(model::Game& game)
		: app_(game) {}


};
} // namespace http_handler