#pragma once
#include "base.h"

namespace http_handler {
	
class File : public Base {
private:
	const fs::path static_path_;

public:
	File(const fs::path& static_path)
		: static_path_{ CheckStaticPath(static_path) } {}

private: 
    TypeRequest parse_target(std::string_view target, std::string& res) const;
	FileRequestResult MakeGetResponse(const StringRequest& req, bool with_body) const;
	FileRequestResult MakePostResponse(const StringRequest& req);
	static fs::path CheckStaticPath(const fs::path& path_static);
	FileRequestResult StaticFilesResponse(
		std::string_view responseText, bool with_body,
		unsigned http_version, bool keep_alive) const;
	bool CheckFileExist(std::string& file) const;
	bool FileInRootStaticDir(std::string_view file) const;
};
} // namespace http_handler
