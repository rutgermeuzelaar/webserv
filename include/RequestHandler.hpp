#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP
# include <optional>
# include <filesystem>
# include "ServerContext.hpp"

class Response;
class Request;

const std::string get_mime_type(const std::filesystem::path& extension);
const std::string create_directory_listing(const std::filesystem::path& directory);
const LocationContext* find_location(const std::string& folder_path, const ServerContext& config);
Response build_error_page(HTTPStatusCode status_code, const LocationContext* location, const ServerContext& config);
bool is_cgi_request(const std::string& uri);
bool request_method_allowed(const LocationContext* location, HTTPMethod method);

class RequestHandler
{
	private:
		const ServerContext& m_config;
		Response handle_get(const LocationContext* location, std::filesystem::path& local_path);
		Response handle_post(const Request& request, const UploadStore& upload_store);
		Response handle_delete(std::filesystem::path& local_path);

	public:
		RequestHandler(const ServerContext&);
		std::filesystem::path map_uri(std::string uri, const LocationContext* location);
		Response handle(const Request& request, const std::string& uri, const LocationContext* location);
};
#endif