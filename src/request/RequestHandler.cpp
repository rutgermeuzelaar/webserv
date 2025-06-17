/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:17:11 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 13:36:54 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <chrono>
#include <cassert>
#include <filesystem>
#include <format>
#include <sys/types.h>
#include <dirent.h>
#include <sstream>
#include "RequestHandler.hpp"
#include "Response.hpp"

static std::string create_header(const std::filesystem::path& directory)
{
    std::string header = std::format(
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
        "<head>"
		    "<meta charset=\"utf-8\"/>"
            "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
		    "<title>Index of {}</title>"
        "</head>"
        , directory.string()
    );
    
    return header;
}

static std::string create_table_rows(const std::filesystem::path& directory)
{
    std::string table_rows;

    for (const auto& item: std::filesystem::directory_iterator(directory))
    {
        const std::string item_path = item.path();
        const auto file_time = std::filesystem::last_write_time(item.path());

        std::string item_name;
        std::string file_size = "-";

        if (!item.is_directory())
        {
            file_size = std::to_string(std::filesystem::file_size(item.path()));
        }
        if (item.path().has_filename())
        {
            item_name = item.path().filename().string();
        }
        else
        {
            item_name = item.path().parent_path().string();
        }
        table_rows += std::format(
            "<tr>"
                "<td><a href=\"{}\">{}</a></td>"
                "<td>{}</td>"
                "<td>{}</td>"
            "</tr>"
            , item_path
            , item_name
            , std::format("{0:%d}-{0:%m}-{0:%Y} {0:%R}", file_time)
            , file_size
        );
    }
    return table_rows;
}

static std::string create_body(const std::filesystem::path& directory)
{
    std::string body = std::format(
        "<body>"
            "<h1>index of {}</h1>"
            "<hr>"
            "<table>"
            "<tr>"
                "<th>name</th>"
                "<th>last modified</th>"
                "<th>size</th>"
            "</tr>"
            "{}" // table rows go here
            "</table>"
            "<hr>"
        "</body>"
    "</html>"
    , directory.string()
    , create_table_rows(directory));

    return body;
}

const std::string create_directory_listing(const std::filesystem::path& directory)
{
    std::string directory_listing = create_header(directory) + create_body(directory);

    return directory_listing;
}

static bool is_jpeg(const std::filesystem::path& extension)
{
    if (extension == ".jpg") return true;    
    if (extension == ".jpeg") return true;    
    if (extension == ".jpe") return true;    
    if (extension == ".jfif") return true;    
    if (extension == ".jif") return true;
    return (false);   
}

const std::string get_mime_type(const std::filesystem::path& extension)
{
    if (extension == ".css") return "text/css";
    if (extension == ".html") return "text/html";
    if (extension == ".ico") return "image/x-icon";
    if (is_jpeg(extension)) return "image/jpeg";
    if (extension == ".png") return "image/png";
    return "text/plain";
}

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

static std::filesystem::path map_uri_helper(std::filesystem::path root_path, std::filesystem::path& uri_path)
{
	if (uri_path.is_absolute())
	{
		uri_path = uri_path.relative_path();
	}
	resolve_overlap(root_path, uri_path);
	return root_path;
}

static bool has_index_html(std::filesystem::path uri)
{
    uri.append("index.html");
    return (std::filesystem::exists(uri));
}

// should find the most specific location
const LocationContext* RequestHandler::find_location(const std::string& folder_path) const
{
    size_t uri_match_len;
    const LocationContext* match = nullptr;

    uri_match_len = 0;
    for (auto& it :m_config.m_location_contexts)
    {
        if (folder_path.compare(0, it.m_uri.size(), it.m_uri) == 0)
        {
            if (it.m_uri.size() > uri_match_len)
            {
                match = &it;
                uri_match_len = it.m_uri.size();
            }
        }
    }
    return match;
}

std::filesystem::path RequestHandler::map_uri(std::string uri, const LocationContext* location)
{
	std::filesystem::path uri_path(uri);
	
    if (location != nullptr && location->m_root.has_value())
    {
        return map_uri_helper(location->m_root.value().m_path, uri_path);
    }
	return map_uri_helper(m_config.m_root.value().m_path, uri_path);
}

// should be expanded with error handling from the config file
static Response build_404(void)
{
    Response response(HTTPStatusCode::NotFound);
	response.setBodyFromFile("./root/pages/404.html");
    response.setContentType("text/html");
	return response;
}

static Response build_redirect(const Return& return_obj)
{
    Response response(return_obj.m_status_code);
    response.setHeader("Location", return_obj.m_uri);
    return response;
}

Response RequestHandler::handle_get(const Request& request)
{
	const std::string& uri = request.getURI();
    const LocationContext* location = find_location(uri);
	std::filesystem::path local_path = map_uri(uri, location);

    if (location != nullptr && location->m_return.has_value())
    {
        return build_redirect(*location->m_return);
    }
    if (std::filesystem::is_directory(local_path))
    {
        if (has_index_html(local_path))
        {
            local_path.append("index.html");
        }
        else if (m_config.m_auto_index.value().m_on)
        {
            // create directory listing
            Response response(HTTPStatusCode::OK);
            response.setBody(create_directory_listing(local_path));
            response.setContentType("text/html");
            return response;
        }
        else
        {
            return build_404();
        }
    }
	if (!std::filesystem::exists(local_path))
	{
        return build_404();
	}
    // directory request
	Response response(HTTPStatusCode::OK);
	response.setBodyFromFile(local_path);
    response.setContentType(get_mime_type(local_path.extension()));
    response.setHeader("Content-Disposition", "inline");
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
            break;
	}
    throw std::runtime_error("Unsupported HTTP method.");
}
