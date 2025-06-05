/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:17:11 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/05 18:02:11 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <filesystem>
#include "RequestHandler.hpp"
#include "Response.hpp"

RequestHandler::RequestHandler(const Config& config)
	: m_config {config}
{

}

std::optional<std::string> RequestHandler::map_uri(std::string uri)
{
	const ServerContext& server = m_config.m_http_context.m_servers.m_vector.at(0);
	std::string filename;
	// get server
	if (server.m_location_contexts.m_vector.size() == 0)
	{
		// defer to default value
	}
	if (uri.back() == '/')
	{
		// hardcode index.html
		uri.append("index.html");
	}
	const size_t last_slash_pos = uri.rfind('/');
	const std::string folder_path = uri.substr(0, last_slash_pos);
	// if received URI like this '/' '/dir/' we look for an index.html file
	// if it doesn't exist check if autoindex is enabled, if it isn't error otherwise generate directory listing
	filename = uri.substr(last_slash_pos, uri.size() - last_slash_pos);
	for (auto& it :server.m_location_contexts.m_vector)
	{
		if (it.m_uri.find(folder_path, 0) == 0)
		{
			// check if it has root block
			if (it.m_root.has_value())
			{
				std::string root_path = it.m_root.value().m_path.string();
				root_path.append(filename);
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
	m_config.m_http_context.m_servers.m_vector.at(0);
	std::optional<std::string> local_path = map_uri(uri);
	if (local_path.has_value())
	{
		Response response(HTTPStatusCode::OK);
		response.setBodyFromFile(local_path.value());
		std::cout << map_uri(uri).value();
		return response;
	}
	else
	{
		std::cout << "Could not map URI.";
		Response response(HTTPStatusCode::NotFound);
		return response;
		// create 
	}
	// if file exists
	 // if file exists but you don't have permissions
	// if file does not exist
}

// for now URI and method only
void RequestHandler::handle(const Request& request, int client_fd)
{
	(void)client_fd;
	switch (request.getMethodType())
	{
		case HTTPMethod::GET:
		{
			
		}
		return;
		case HTTPMethod::DELETE:
		return;
		case HTTPMethod::POST:
		return;
		default:
		assert(false);
	}
}