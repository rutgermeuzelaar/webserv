#ifndef HTTP_STATUS_LINE_HPP
# define HTTP_STATUS_LINE_HPP
# include <sstream>
# include <string>
# include "HTTPStatusCode.hpp"

class HTTPStatusLine
{
    private:
        static std::string    m_protocol;
        std::string           m_status_text;           
        HTTPStatusCode              m_status_code;

    public:
        HTTPStatusLine(HTTPStatusCode status_code);
		HTTPStatusLine(const HTTPStatusLine& src);
        HTTPStatusLine& operator=(const HTTPStatusLine& other);
        
        //* getters
        const std::string& getStatusText() const { return m_status_text; }
        HTTPStatusCode getStatusCode() const { return m_status_code; }
        const std::string& getProtocol() const { return m_protocol; }
};

std::ostream& operator<<(std::ostringstream&, const HTTPStatusLine& http_status_line);
#endif