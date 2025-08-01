#include "Pch.hpp"
#include <cassert>
#include <sys/stat.h>
#include "RequestHandler.hpp"
#include "Utilities.hpp"
#include "MIMETypes.hpp"
#include "Response.hpp"
#include "Request.hpp"

static std::string create_header(const std::filesystem::path& directory)
{
    std::string header = std::string(
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
        "<head>"
		    "<meta charset=\"utf-8\"/>"
            "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
		    "<title>Index of ") + directory.string() + std::string("</title>"
        "</head>");
    
    return header;
}

static std::string create_table_rows(const std::filesystem::path& directory)
{
    std::string table_rows;

    for (const auto& item: std::filesystem::directory_iterator(directory))
    {
        const std::string item_path = item.path();
		// file_time is annoying to print if we can't use
		// C++ 20
		struct stat attributes;

		std::string time_stamp;

		if (stat(item_path.c_str(), &attributes) == -1)
		{
			throw (std::runtime_error("stat"));
		}
		time_stamp = std::ctime(&attributes.st_mtime);
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
        table_rows += std::string(
            "<tr>"
                "<td><a href=\"") + std::string(item_path).erase(0, 1) + std::string("\">") + item_name + \
				std::string("</a></td>"
                "<td>") + time_stamp + std::string ("</td>"
                "<td>") + file_size + std::string("</td>"
            "</tr>");
    }
    return table_rows;
}

static std::string create_body(const std::filesystem::path& directory)
{
    std::string body = std::string(
        "<body>"
            "<h1>index of ") + directory.string() + std::string("</h1>"
            "<hr>"
            "<table>"
            "<tr>"
                "<th>name</th>"
                "<th>last modified</th>"
                "<th>size</th>"
            "</tr>") + create_table_rows(directory) + std::string(
            "</table>"
            "<hr>"
        "</body>"
    "</html>");
    
	return body;
}

const std::string create_directory_listing(const std::filesystem::path& directory)
{
    std::string directory_listing = create_header(directory) + create_body(directory);

    return directory_listing;
}

const std::string get_mime_type(const std::string& extension)
{
    for (auto it: g_mime_types)
    {
        if (std::find(it.second.begin(), it.second.end(), extension) != it.second.end())
        {
            return it.first;
        }
    }
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

static void find_page(const Index& index, std::string& buffer, std::filesystem::path uri)
{
    for (auto it: index.m_files)
    {
        std::filesystem::path uri_cp = uri;
        std::error_code ec;
        if (std::filesystem::exists(uri_cp.append(it), ec))
        {
            buffer = it;
            return;
        }
        else if (ec.value() == EACCES)
        {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
    }
}

// should find the most specific location
const LocationContext* find_location(const std::string& folder_path, const ServerContext& config)
{
    size_t uri_match_len;
    const LocationContext* match = nullptr;

    uri_match_len = 0;
    for (auto& it :config.m_location_contexts)
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

std::filesystem::path map_uri(std::string uri, const LocationContext* location, const Root& root)
{
	std::filesystem::path uri_path(uri);
	
    if (location != nullptr && location->m_root.has_value())
    {
        return map_uri_helper(location->m_root.value().m_path, uri_path);
    }
	return map_uri_helper(root.m_path, uri_path);
}

static Response build_redirect(const Return& return_obj)
{
    Response response(return_obj.m_status_code);
    response.setHeader("Location", return_obj.m_uri);
    response.setHeader("Content-Length", "0");
	response.finalize();
    return response;
}

static const std::string create_dynamic_error_page(HTTPStatusCode status_code)
{
    const std::string status_text = get_http_status_text(status_code);
    const std::string error_page = std::string(
    "<!DOCTYPE html>"
    "<html lang=\"en-US\">"
        "<head>"
            "<meta charset=\"utf-8\"/>"
            "<link rel=\"stylesheet\" href=\"" STYLESHEET "\">"
            "<title>") + status_text + std::string("</title>"
        "</head>"
        "<body>"
            "<h1>") + std::to_string(static_cast<int>(status_code)) + std::string(" - ") + status_text + std::string("</h1>"
            "<p><a href=\"/\">Home</a></p>"
        "</body>"
    "</html>");
    
	return error_page;
}

Response build_error_page(HTTPStatusCode status_code, const LocationContext* location, const ServerContext& config)
{
    Response response(status_code);

    response.setContentType("text/html");
    if (location != nullptr)
    {
        for (auto& it: location->m_error_pages)
        {
            if (it.m_status_code == status_code)
            {
                response.setBodyFromFile(it.m_path);
                return response;
            }
        }
    }
    for (auto& it: config.m_error_pages)
    {
        if (it.m_status_code == status_code)
        {
            response.setBodyFromFile(it.m_path);
            return response;
        }
    }
    response.setBody(create_dynamic_error_page(status_code));
    return response;
}

Response RequestHandler::handle_get(const LocationContext* location, std::filesystem::path& local_path)
{
    if (location != nullptr && location->m_return.has_value())
    {
        return build_redirect(*location->m_return);
    }
    if (std::filesystem::is_directory(local_path))
    {
        std::string page_name_buf;

        if (location == nullptr || !location->m_index.has_value())
        {
            find_page(m_config.m_index.value(), page_name_buf, local_path);
        }
        else
        {
            find_page(location->m_index.value(), page_name_buf, local_path);
        }
        if (!page_name_buf.empty())
        {
            local_path.append(page_name_buf);
        }
        else if (location != nullptr && location->m_auto_index.has_value())
        {
            if (location->m_auto_index.value().m_on)
            {
                Response response(HTTPStatusCode::OK);
                response.setBody(create_directory_listing(local_path));
                response.setContentType("text/html");
                return response;
            }
            return build_error_page(HTTPStatusCode::NotFound, location, m_config);
        }
        else if (m_config.m_auto_index.value().m_on)
        {
            Response response(HTTPStatusCode::OK);
            response.setContentType("text/html");
            response.setBody(create_directory_listing(local_path));
            return response;
        }
        else
        {
            return build_error_page(HTTPStatusCode::NotFound, location, m_config);
        }
    }
	if (!std::filesystem::exists(local_path))
	{
        return build_error_page(HTTPStatusCode::NotFound, location, m_config);
	}
	Response response(HTTPStatusCode::OK);  
    response.setContentType(get_mime_type(local_path.extension().string()));
    response.setHeader("Content-Disposition", "inline");
    response.setBodyFromFile(local_path);
	return response;
}

Response RequestHandler::handle_delete(std::filesystem::path& local_path)
{
    std::error_code ec;
    if (!std::filesystem::exists(local_path, ec))
    {
        if (!ec)
        {
            throw HTTPException(HTTPStatusCode::NotFound);
        }
        if (ec.value() == EACCES)
        {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (!std::filesystem::remove(local_path, ec))
    {
        if (ec.value() == EACCES)
        {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
		if (ec.value() == ENOTEMPTY)
		{
			throw HTTPException(HTTPStatusCode::Conflict);
		}
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    Response response(HTTPStatusCode::NoContent);
    return response;
}

static const std::string get_extension(const std::string& mime_type)
{
    auto pos = g_mime_types.find(mime_type);
    if (pos == g_mime_types.end())
    {
        throw HTTPException(HTTPStatusCode::UnsupportedMediaType);   
    }
    if (pos->second.size() > 0)
    {
        return pos->second[0];
    }
    return "";
}

Response RequestHandler::handle_post(const Request& request, const UploadStore& upload_store)
{
    const std::string& content_type = request.getHeaders().get_header("content-type");
    std::string path = upload_store.m_path;
    std::string file_name;
    std::string file_data;
    std::string extension;

    if (content_type.find("multipart/form-data") != std::string::npos) 
	{
        const MultiPartChunk& chunk = request.getBody().get_multi_part_chunk();
		const std::string& mime_type = chunk.get_mime_type();
		if (g_mime_types.find(mime_type) == g_mime_types.end())
			throw HTTPException(HTTPStatusCode::UnsupportedMediaType);
        extension = get_extension(chunk.get_mime_type());
        file_name = create_file_name(extension);
        file_data = chunk.m_data;
    } 
	else 
	{
        file_data = request.getBody().get_raw();
		if (g_mime_types.find(content_type) == g_mime_types.end())
			throw HTTPException(HTTPStatusCode::UnsupportedMediaType);
		extension = get_extension(content_type);
        file_name = create_file_name(extension);
    }

    if (ends_with(path, "/"))
        path.append(file_name);
    else {
        path.append("/");
        path.append(file_name);
    }
    std::ofstream file(path, std::ios::binary);
    if (file.fail()) {
        if (errno == EACCES) {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
	std::cout << "file_data size:"  << file_data.size() << std::endl;
	// std::cout << "file_data content: " << file_data << std::endl;
    file << file_data;
    file.close();
    Response response(HTTPStatusCode::SeeOther);
    response.setHeader("Location", path);
    response.setHeader("Content-Length", "0");
	response.finalize();
    return response;
}

bool is_cgi_request(const std::string& uri)
{
    const std::string cgi_str = "/" CGI_DIR "/";

    size_t str_pos = uri.find("/" CGI_DIR "/", 0);

    if (str_pos == std::string::npos)
    {
        return (false);
    }
    if (str_pos + cgi_str.size() == uri.size())
    {
        return (false);
    }
    return (true);
}

bool request_method_allowed(const LocationContext* location, HTTPMethod method)
{   
    if (location != nullptr && location->m_limit_except.has_value())
    {
        const auto& allowed = location->m_limit_except.value().m_allowed_methods;

        if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
        {
            return (false);
        }
    }
    return (true);
}

Response RequestHandler::handle(const Request& request, const std::string& uri, const LocationContext* location)
{
    const HTTPMethod method = request.getStartLine().get_http_method();
	std::filesystem::path local_path = map_uri(uri, location, m_config.m_root.value());

    if (!request_method_allowed(location, method))
    {
        return build_error_page(HTTPStatusCode::MethodNotAllowed, location, m_config);
    }
    switch (method)
    {
        case HTTPMethod::GET:
            return handle_get(location, local_path);
        case HTTPMethod::DELETE:
            return handle_delete(local_path);
        case HTTPMethod::POST:
            if (location == nullptr || !location->m_upload_store.has_value())
            {
                return handle_post(request, m_config.m_upload_store.value());
            }
            else
            {
                return handle_post(request, location->m_upload_store.value());
            }
        default:
            return build_error_page(HTTPStatusCode::BadRequest, location, m_config);
    }
}
