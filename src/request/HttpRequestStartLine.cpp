#include "Pch.hpp" // IWYU pragma: keep
#include <vector>
#include <string>

#include "Utilities.hpp"
#include "Defines.hpp"
#include "Http.hpp"

HttpRequestStartLine::HttpRequestStartLine()
    : m_complete {false}
{

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
    m_uri = check_uri(args[1]);
    if (args[2] == PROTOCOL)
    {
        m_complete = true;
        return;
    }
    throw HTTPException(HTTPStatusCode::HTTPVersionNotSupported);
}

/**
 * @brief check if Uniform Resource Identifier (URI) from the request line is valid.
 * @param uri alias of the raw URI
 * @return uri return reference to URI if valid
 */
const std::string& HttpRequestStartLine::check_uri(const std::string& uri)
{
	if (uri.empty() || uri[0] != '/')
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid URI format");
	if (uri.length() > MAX_URI_SIZE)
		throw HTTPException(HTTPStatusCode::URITooLong, "URI exceeds maximum length");
	return uri;
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
