#include "Pch.hpp"
#include "Response.hpp"
#include <ctime>
#include <iomanip>
#include <cassert>
#include <iterator>
#include "Utilities.hpp"

Response::Response(HTTPStatusCode status_code)
    : _status_line(status_code)
    , _bytes_sent (0)
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
	: _status_line(src._status_line)
	, _headers {src._headers}
	, _body {src._body}
    , _bytes (src._bytes)
    , _bytes_sent (src._bytes_sent)
    , _client_fd (src._client_fd)
    , _headers_complete (false)
    , _headers_size (0)
    , _body_size (0)
{

}

Response& Response::operator=(const Response& src)
{
    _status_line = src._status_line;
    _headers = src._headers;
    _body = src._body;
    _bytes_sent = src._bytes_sent;
    _bytes = src._bytes;
    _client_fd = src._client_fd;
    _headers_complete = src._headers_complete;
    _headers_size = src._headers_size;
    _body_size = src._body_size;
    return *this;
}

Response::Response(Response&& other) noexcept
    : _status_line {std::move(other._status_line)}
    , _headers {std::move(other._headers)}
    , _body {std::move(other._body)}
    , _bytes {std::move(other._bytes)}
    , _bytes_sent {std::move(other._bytes_sent)}
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
    assert("You cannot add a header after a body is set" && !_headers_complete);
    if (key.empty())
        throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
    
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    _headers[lowerKey] = value;
}

void Response::setBody(const std::string& content)
{
    setHeader("Content-Length", std::to_string(content.length()));
    _headers_complete = true;
    headersToBytes();
    _body = content;
    copy_str_bytes(_bytes, content);
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
    _bytes.reserve(file_size + MAX_HEADER_SIZE);
    setHeader("Content-Length", std::to_string(file_size));
    _headers_complete = true;
    headersToBytes();
    std::vector<std::byte> temp(file_size);
    file.read(reinterpret_cast<char *>(temp.data()), file_size);
    _bytes.insert(
        _bytes.end(),
        temp.begin(),
        temp.end()
    );
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
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    std::map<std::string, std::string>::const_iterator headerIt = _headers.find(lowerKey);
    if (headerIt != _headers.end())
        return headerIt->second;
    return "";
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

const std::byte* Response::getNextBytes(size_t *length)
{
    const size_t bytes_length = _bytes.size() - _bytes_sent;

    *length = bytes_length;
    return (_bytes.data() + _bytes_sent);
}

void Response::incrementBytesSent(size_t amount)
{
    _bytes_sent += amount;
}

bool Response::fullySent(void) const
{
    return _bytes_sent == _bytes.size();
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
	assert(_bytes.empty());
    std::ostringstream stream;

    stream << _status_line << LINE_BREAK;
    for (const auto& header : _headers)
    {
        stream << header.first << ": " << header.second << LINE_BREAK;
    }
    stream << LINE_BREAK;
    copy_str_bytes(_bytes, stream.str());
    _headers_size = _bytes.size();
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
        std::putchar(static_cast<char>(_bytes[i]));
    }
}

void Response::printBody(void) const
{
    for (size_t i = _headers_size; i < _bytes.size(); ++i)
    {
        std::putchar(static_cast<char>(_bytes[i]));
    }
}
