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
                "<td><a href=\"") + item_path + std::string("\">") + item_name + \
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

static void find_page(const Index& index, std::string& buffer, std::filesystem::path uri)
{
    for (auto it: index.m_files)
    {
        std::filesystem::path uri_cp = uri;
        if (std::filesystem::exists(uri_cp.append(it)))
        {
            buffer = it;
            return;
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

std::filesystem::path RequestHandler::map_uri(std::string uri, const LocationContext* location)
{
	std::filesystem::path uri_path(uri);
	
    if (location != nullptr && location->m_root.has_value())
    {
        return map_uri_helper(location->m_root.value().m_path, uri_path);
    }
	return map_uri_helper(m_config.m_root.value().m_path, uri_path);
}

static Response build_redirect(const Return& return_obj)
{
    Response response(return_obj.m_status_code);
    response.setHeader("Location", return_obj.m_uri);
    response.setHeader("Content-Length", "0");
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
            "<link rel=\"stylesheet\" href=\"../css/stylesheet.css\">"
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

Response RequestHandler::handle_get(const std::string& uri, const LocationContext* location)
{
	std::filesystem::path local_path = map_uri(uri, location);

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
            return build_error_page(HTTPStatusCode::NotFound, location, m_config);
        }
    }
	if (!std::filesystem::exists(local_path))
	{
        return build_error_page(HTTPStatusCode::NotFound, location, m_config);
	}
	Response response(HTTPStatusCode::OK);  
	response.setBodyFromFile(local_path);
    response.setContentType(get_mime_type(local_path.extension()));
    response.setHeader("Content-Disposition", "inline");
	return response;
}

Response RequestHandler::handle_delete(const Request& request)
{
    (void)request;
    return Response();
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
    const MultiPartChunk& chunk = request.getBody().get_multi_part_chunk();
    const std::string file_name = create_file_name(get_extension(chunk.get_mime_type()));
    std::string path = upload_store.m_path;

    if (ends_with(path, "/"))
    {
        path.append(file_name);
    }
    else
    {
        path.append("/");
        path.append(file_name);
    }
    std::ofstream file(path);
    if (file.fail())
    {
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    file << chunk.m_data;
    file.close();
    Response response(HTTPStatusCode::SeeOther);
    response.setHeader("Location", path);
    response.setHeader("Content-Length", "0");
    return response;
}

// for now URI and method only
Response RequestHandler::handle(const Request& request)
{
	const std::string& uri = request.getStartLine().get_uri();
    const LocationContext* location = find_location(uri, m_config);
    const HTTPMethod method = request.getStartLine().get_http_method();

    if (location != nullptr && location->m_limit_except.has_value())
    {
        const auto& allowed = location->m_limit_except.value().m_allowed_methods;

        if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
        {
            return build_error_page(HTTPStatusCode::MethodNotAllowed, location, m_config);
        }
    }
    switch (method)
    {
        case HTTPMethod::GET:
            return handle_get(uri, location);
        case HTTPMethod::DELETE:
            return handle_delete(request);
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
