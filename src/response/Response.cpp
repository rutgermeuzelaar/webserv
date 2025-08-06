#include "Pch.hpp"
#include <ctime>
#include <iomanip>
#include <cassert>
#include <iterator>
#include <fstream>
#include "Defines.hpp"
#include "Utilities.hpp"
#include "Http.hpp"

Response::Response(HTTPStatusCode status_code)
    : PartialWriter()
    , _status_line(status_code)
    , _client_fd (0)
    , _headers_complete (false)
    , _headers_size (0)
    , _body_size (0)
{
    //* set default headers
    setContentType("text/html");
    setServer();
    setDate();
}

Response::Response(const Response& src)
    : PartialWriter(src)
    , _status_line {src._status_line}
	, _headers {src._headers}
	, _body {src._body}
    , _client_fd {src._client_fd}
    , _headers_complete {false}
    , _headers_size {0}
    , _body_size {0}
{

}

Response& Response::operator=(const Response& src)
{
    PartialWriter::operator=(src);
    _status_line = src._status_line;
    _headers = src._headers;
    _body = src._body;
    _client_fd = src._client_fd;
    _headers_complete = src._headers_complete;
    _headers_size = src._headers_size;
    _body_size = src._body_size;
    return *this;
}

Response::Response(Response&& other) noexcept
    : PartialWriter(std::move(other))
    , _status_line {std::move(other._status_line)}
    , _headers {std::move(other._headers)}
    , _body {std::move(other._body)}
    , _client_fd {std::move(other._client_fd)}
    , _headers_complete {std::move(other._headers_complete)}
    , _headers_size {std::move(other._headers_size)}
    , _body_size {std::move(other._body_size)}
{

}

Response::~Response()
{}

void Response::setHeader(const std::string& key, const std::string& value)
{
    if (key.empty())
        throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
    
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    _headers.add_header(lowerKey, value);
    // need to insert bytes here
    if (_headers_complete)
    {
        std::vector<std::byte> temp;
        temp.reserve(key.size() + value.size() + 4);
        copy_str_bytes(temp, lowerKey);
        copy_str_bytes(temp, ": ");
        copy_str_bytes(temp, value);
        copy_str_bytes(temp, LINE_BREAK);
        // status_line size + current headers size
        auto& bytes = get_bytes();
        bytes.insert((bytes.begin() + _headers_size - 2), temp.begin(), temp.end());
        _headers_size += temp.size();
    }
}

void Response::setBody(const std::string& content)
{
    setHeader("Content-Length", std::to_string(content.length()));
    _headers_complete = true;
    headersToBytes();
    _body = content;
    append_bytes(content);
    _body_size = content.length();
}

void Response::setBodyFromFile(const std::filesystem::path& file_path)
{
    assert(std::filesystem::exists(file_path));
    const size_t file_size = std::filesystem::file_size(file_path);

    std::ifstream file(file_path);

    if (!file.is_open())
    {
        if (errno == EACCES)
        {
            throw HTTPException(HTTPStatusCode::Forbidden);
        }
        else if (errno == ENOENT)
        {
            throw HTTPException(HTTPStatusCode::NotFound, "File not found: " + file_path.string());
        }
        throw HTTPException(HTTPStatusCode::InternalServerError);
    } 
    get_bytes().reserve(file_size + MAX_HEADER_SIZE);
    setHeader("Content-Length", std::to_string(file_size));
    _headers_complete = true;
    headersToBytes();
    std::vector<std::byte> temp(file_size);
    file.read(reinterpret_cast<char *>(temp.data()), file_size);
    append_bytes(temp);
    if (file.fail())
        throw HTTPException(HTTPStatusCode::InternalServerError, "Failed to read file: " + file_path.string());
    _body_size = file_size;
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
    return _headers.get_header(key);
}

void Response::printResponse() const
{
    std::cout << "\n--- Response Details ---" << std::endl; 
    std::cout << "\nHeaders:" << std::endl;
    printHeaders();
    std::cout << "\nBody:" << std::endl;
    printBody();
    std::cout << "---------------------\n" << std::endl;
}

void Response::setClientFD(int client_fd)
{
    _client_fd = client_fd;
}

int Response::getClientFD(void) const
{
    return _client_fd;
}

void Response::headersToBytes(void)
{
    assert(_headers_complete);
	assert(get_bytes().empty());
    std::ostringstream stream;

    stream << _status_line << LINE_BREAK;
    const auto& headers = _headers.get_headers();
    for (const auto& header: headers)
    {
        stream << header.first << ": " << header.second << LINE_BREAK;
    }
    stream << LINE_BREAK;
    append_bytes(stream.str());
    _headers_size = get_bytes().size();
}

size_t Response::getBodySize(void) const
{
    return _body_size;
}

size_t Response::getHeadersSize(void) const
{
    return _headers_size;
}

void Response::printHeaders(void) const
{
    for (size_t i = 0; i < _headers_size; ++i)
    {
        std::putchar(static_cast<char>(get_bytes()[i]));
    }
}

void Response::printBody(void) const
{
    for (size_t i = _headers_size; i < get_bytes().size(); ++i)
    {
        std::putchar(static_cast<char>(get_bytes()[i]));
    }
}

void Response::finalize(void)
{
	_headers_complete = true;
	headersToBytes();
}

bool Response::getHeadersComplete(void) const
{
	return _headers_complete;
}

void Response::setStatusLine(HTTPStatusCode status_code)
{
    _status_line = HTTPStatusCode(status_code);
}

HttpHeaders& Response::getHeaders()
{
    return _headers;
}
