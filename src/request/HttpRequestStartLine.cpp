#include <vector>
#include "HttpRequestStartLine.hpp"
#include "Utilities.hpp"
#include "Defines.hpp"
#include "HTTPException.hpp"

HttpRequestStartLine::HttpRequestStartLine()
    : m_complete {false}
{

}

HttpRequestStartLine& HttpRequestStartLine::operator=(const HttpRequestStartLine& http_request_start_line)
{
    m_http_method = http_request_start_line.m_http_method;
    m_uri = http_request_start_line.m_uri;
    m_version = http_request_start_line.m_version;
    m_complete = http_request_start_line.m_complete;
    return *this;
}

HttpRequestStartLine::~HttpRequestStartLine()
{

}

HttpRequestStartLine::HttpRequestStartLine(std::string str)
{
    if (!ends_with(str, LINE_BREAK))
    {
        throw HTTPException(HTTPStatusCode::BadRequest);
    }
    std::vector<std::string> args = split(rtrim(str, LINE_BREAK), ' ');
    m_http_method = StringToMethod(args[0]);
    m_uri = parse_uri(args[1]);
    if (args[2] == PROTOCOL)
    {
        m_complete = true;
        return;
    }
    throw HTTPException(HTTPStatusCode::HTTPVersionNotSupported);
}

/**
 * @brief parses Uniform Resource Identifier (URI) from the request line.
 * @param uri alias of the raw uri to parse
 * @note query is described after the `?`, and a framework after a `#`
 * @note example URI: https://www.example.com/forum/questions/?tag=networking&order=newest#top
 * @return true if URI is valid and parsing succeeded, false if URI is invalid
 */
const std::string& HttpRequestStartLine::parse_uri(const std::string& uri)
{
	if (uri.empty() || uri[0] != '/')
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid URI format");
	if (uri.length() > MAX_URI_SIZE)
		throw HTTPException(HTTPStatusCode::URITooLong, "URI exceeds maximum length");
	return uri;

    // Responsibility of the request handler I think?
	// size_t queryPos = uri.find('?');
	// if (queryPos != std::string::npos)
	// {
	// 	_path = uri.substr(0, queryPos);
	// 	size_t fragmentPos = uri.find('#', queryPos);
	// 	if (fragmentPos != std::string::npos)
	// 	{
	// 		_query = uri.substr(queryPos + 1, fragmentPos - queryPos - 1);
	// 		_fragment = uri.substr(fragmentPos + 1);
	// 	}
	// 	else
	// 		_query = uri.substr(queryPos + 1);
	// }
	// else
	// {
	// 	//* no query string, check for fragment
	// 	size_t fragmentPos = uri.find('#');
	// 	if (fragmentPos != std::string::npos)
	// 	{
	// 		_path = uri.substr(0, fragmentPos);
	// 		_fragment = uri.substr(fragmentPos + 1);
	// 	}
	// 	else
	// 		_path = uri;
	// }
}

bool HttpRequestStartLine::complete() const
{
    return m_complete;
}

HTTPMethod HttpRequestStartLine::get_http_method(void) const
{
    return m_http_method;
}

const std::string& HttpRequestStartLine::get_uri(void) const
{
    return m_uri;
}

const std::string& HttpRequestStartLine::get_version(void) const
{
    return m_version;
}