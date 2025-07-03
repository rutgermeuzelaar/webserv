#ifndef HTTP_REQUEST_START_LINE_HPP
# define HTTP_REQUEST_START_LINE_HPP
# include <string>
# include "HttpMethod.hpp"

class HttpRequestStartLine
{
    private:
        HTTPMethod  m_http_method;
        std::string m_uri;
        std::string m_version;
        bool        m_complete;

        const std::string& parse_uri(const std::string& uri);
    public:
        HttpRequestStartLine(void);
        ~HttpRequestStartLine(void);
        friend void swap(HttpRequestStartLine&, HttpRequestStartLine&) noexcept;
        HttpRequestStartLine& operator=(HttpRequestStartLine);
        HttpRequestStartLine(std::string);

        HTTPMethod get_http_method(void) const;
        const std::string& get_uri(void) const;
        const std::string& get_version(void) const;
        bool complete(void) const;
};
#endif