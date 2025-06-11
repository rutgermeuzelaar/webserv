/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:17:11 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 19:51:31 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <filesystem>
#include "RequestHandler.hpp"
#include "Response.hpp"

RequestHandler::RequestHandler(const ServerContext& config)
	: m_config {config}
{

}

// ./root + ./root/css+stylesheet.css
static std::filesystem::path& resolve_overlap(std::filesystem::path& root, const std::filesystem::path& to_join)
{
	std::filesystem::path::iterator root_it = root.begin();
	std::filesystem::path::iterator join_it = to_join.begin();

	if (*root_it == ".")
	{
		root_it++;
	}
	while (root_it != root.end() && join_it != to_join.end() && *root_it == *join_it)
	{
		root_it++;
		join_it++;
	}
	while (join_it != to_join.end())
	{
		root /= *join_it;
		join_it++;
	}
	return root;
}

std::optional<std::string> RequestHandler::map_uri(std::string uri)
{
	std::string filename;
	std::filesystem::path uri_path(uri);
	// get server
	if (m_config.m_location_contexts.size() == 0)
	{
		// defer to default value
	}
	if (uri_path.string().back() == '/')
	{
		// hardcode index.html
		uri_path.append("index.html");
	}
	const std::string folder_path = uri_path.parent_path();
	// if received URI like this '/' '/dir/' we look for an index.html file
	// if it doesn't exist check if autoindex is enabled, if it isn't error otherwise generate directory listing
	// filename = uri.substr(last_slash_pos, uri.size() - last_slash_pos);
	for (auto& it :m_config.m_location_contexts)
	{
		if (folder_path.compare(0, it.m_uri.size(), it.m_uri) == 0)
		{
			// check if it has root block
			if (it.m_root.has_value())
			{
				std::filesystem::path root_path = it.m_root.value().m_path;
				if (uri_path.is_absolute())
				{
					uri_path = uri_path.relative_path();
				}
				resolve_overlap(root_path, uri_path);
				std::cout << root_path << '\n';
				return std::optional<std::string>(root_path);
			}
		}
	}
	return std::nullopt;
}

Response RequestHandler::handle_get(const Request& request)
{
	// Response response;
	const std::string& uri = request.getURI();

	// write to the specified connection
	// m_config.m_http_context.m_servers.m_vector.at(0);
	std::optional<std::string> local_path = map_uri(uri);
	if (!local_path.has_value() || !std::filesystem::exists(local_path.value()))
	{
		std::cout << "Could not map URI.";
		Response response(HTTPStatusCode::NotFound);
		response.setBodyFromFile("./root/pages/404.html");
		return response;
	}
	Response response(HTTPStatusCode::OK);
	response.setBodyFromFile(local_path.value());
	std::cout << map_uri(uri).value();
	return response;
		// create 
	// if file exists
	 // if file exists but you don't have permissions
	// if file does not exist
}

Response RequestHandler::handle_delete(const Request& request)
{
    (void)request;
    return Response();
}

Response RequestHandler::handle_post(const Request& request)
{
    (void)request;
    return Response();
}

// for now URI and method only
Response RequestHandler::handle(const Request& request)
{
	switch (request.getMethodType())
	{
		case HTTPMethod::GET:
			return handle_get(request);
		case HTTPMethod::DELETE:
			return handle_delete(request);
		case HTTPMethod::POST:
			return handle_post(request);
        default:
            throw std::runtime_error("Unsupported HTTP method.");
	}
    throw std::runtime_error("Unsupported HTTP method.");
}