#include "Pch.hpp"
#include "Response.hpp"
#include <ctime>
#include <iomanip>

Response::Response(HTTPStatusCode status_code)
    : _status_line(status_code)
{
    //* set default headers
    setContentType("text/html");
    setServer();
    setDate();
}

Response::Response(const Response& src)
	: _status_line(src._status_line)
	, _headers {src._headers}
	, _body {src._body}
	, _raw_response {src._raw_response}
{}

Response::~Response()
{}

void Response::setHeader(const std::string& key, const std::string& value)
{
    if (key.empty())
        throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
    
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    _headers[lowerKey] = value;
}

void Response::setBody(const std::string& content)
{
    _body = content;
    setHeader("Content-Length", std::to_string(content.length()));
}

void Response::setBodyFromFile(const std::string& file_path)
{
    std::ifstream file(file_path);

    if (!file.is_open())
    {
        if (errno == EACCES)
        {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
        else if (errno == ENOENT)
        {
            throw HTTPException(HTTPStatusCode::NotFound, "File not found: " + file_path);
        }
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.fail())
        throw HTTPException(HTTPStatusCode::InternalServerError, "Failed to read file: " + file_path);
    setBody(buffer.str());
}

void Response::redirect(const std::string& location, HTTPStatusCode status_code)
{
    //* create new status line with the given status code
    _status_line = HTTPStatusLine(status_code);
    setHeader("Location", location);
    setBody(""); //* empty body for redirects
}

void Response::setContentType(const std::string& type, const std::string& charset)
{
    std::string value = type;
    if (!charset.empty()) {
        value += "; charset=" + charset;
    }
    setHeader("Content-Type", value);
}

void Response::setDate()
{
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::gmtime(&now);
    std::stringstream ss;
    ss << std::put_time(tm, "%a, %d %b %Y %H:%M:%S GMT");
    setHeader("Date", ss.str());
}

void Response::setLastModified(const std::string& date)
{
    setHeader("Last-Modified", date);
}

void Response::setCacheControl(const std::string& value)
{
    setHeader("Cache-Control", value);
}

void Response::setServer(const std::string& server)
{
    setHeader("Server", server);
}

HTTPStatusCode Response::getStatusCode() const
{
    return _status_line.getStatusCode();
}

std::string Response::getStatusText() const
{
    return _status_line.getStatusText();
}

std::string Response::getHeader(const std::string& key) const
{
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    std::map<std::string, std::string>::const_iterator headerIt = _headers.find(lowerKey);
    if (headerIt != _headers.end())
        return headerIt->second;
    return "";
}

std::string Response::getBody() const
{
    return _body;
}

const std::string& Response::to_str() const
{
    std::ostringstream stream;

    stream << _status_line << LINE_BREAK;
    for (const auto& header : _headers)
    {
        stream << header.first << ": " << header.second << LINE_BREAK;
    }
    //* add blank line to separate headers from body
    stream << LINE_BREAK;
    //* add body (if any)
    if (!_body.empty())
        stream << _body;

    _raw_response = stream.str();
    return _raw_response;
}

void Response::printResponse() const
{
    std::cout << "\n--- Response Details ---" << std::endl;
    std::cout << "Status: " << static_cast<int>(_status_line.getStatusCode()) << " " << _status_line.getStatusText() << std::endl;
    
    std::cout << "\nHeaders:" << std::endl;
    for (const auto& header : _headers)
        std::cout << "  " << header.first << ": " << header.second << std::endl;
    
    std::cout << "\nBody:" << std::endl;
    std::cout << _body << std::endl;
    std::cout << "---------------------\n" << std::endl;
}