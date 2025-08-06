#include "Pch.hpp"
#include <string>
#include "Defines.hpp"
#include "Http.hpp"

std::string HTTPStatusLine::m_protocol = PROTOCOL;

HTTPStatusLine::HTTPStatusLine(HTTPStatusCode status_code)
    : m_status_text {get_http_status_text(status_code)}
    , m_status_code {status_code}
{}

HTTPStatusLine::HTTPStatusLine(const HTTPStatusLine& src)
	: m_status_text {src.m_status_text}
	, m_status_code {src.m_status_code}
{
	
}

HTTPStatusLine& HTTPStatusLine::operator=(const HTTPStatusLine& other)
{
    m_status_text = other.m_status_text;
    m_status_code = other.m_status_code;
    return *this;
}

std::ostream& operator<<(std::ostringstream& ostream, const HTTPStatusLine& http_status_line)
{
    ostream << http_status_line.getProtocol() << ' ' 
            << http_status_line.getStatusCode() << ' ' 
            << http_status_line.getStatusText();
    return ostream;
}
