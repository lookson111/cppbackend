#pragma once
#include "base.h"
#include "../app.h"

namespace http_handler {

class Api : public Base {
private:
	app::App app_;
    virtual FileRequestResult MakeGetResponse(
        const StringRequest& req, bool with_body) const override;
	virtual FileRequestResult MakePostResponse(
        const StringRequest& req) const override;
    TypeRequest ParseTarget(
        std::string_view target, std::string& res) const;

public:
	Api(model::Game& game)
		: app_(game) {}


};
} // namespace http_handler
