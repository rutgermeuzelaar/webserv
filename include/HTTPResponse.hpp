/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/26 20:11:13 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/29 18:47:42 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP
# include <map>
# include <string>
# include <iostream>
# include "HTTPStatusCode.hpp"
# include "HTTPStatusLine.hpp"

class HTTPResponse
{
    private:
        std::string                         m_str;
    public:
        HTTPStatusLine                      m_status_line;
        std::map<std::string, std::string>  m_headers;
        std::string                         m_body;

        HTTPResponse(HTTPStatusCode status_code);
        const std::string& to_str(void);
        bool set_body(const std::string html_path);
};

HTTPResponse respond(HTTPStatusCode status_code, std::map<std::string, std::string> &headers, const std::string html_path);
// std::ostream& operator>>(std::ostream&, const HTTPResponse&);
#endif