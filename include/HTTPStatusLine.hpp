/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatusLine.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 21:27:25 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/04 14:29:16 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_STATUS_LINE_HPP
# define HTTP_STATUS_LINE_HPP
# include <sstream>
# include <string>
# include "HTTPStatusCode.hpp"

class HTTPStatusLine
{
    private:
        static const std::string    m_protocol;
        const std::string           m_status_text;           
        HTTPStatusCode              m_status_code;

    public:
        HTTPStatusLine(HTTPStatusCode status_code);
        HTTPStatusLine& operator=(const HTTPStatusLine& other);
        
        //* getters
        const std::string& getStatusText() const { return m_status_text; }
        HTTPStatusCode getStatusCode() const { return m_status_code; }
        const std::string& getProtocol() const { return m_protocol; }
};

std::ostream& operator<<(std::ostringstream&, const HTTPStatusLine& http_status_line);
#endif