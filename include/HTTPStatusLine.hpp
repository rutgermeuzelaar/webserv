/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatusLine.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 21:27:25 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/28 22:38:45 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_STATUS_LINE_HPP
# define HTTP_STATUS_LINE_HPP
# include <sstream>
# include <string>
# include "HTTPStatusCode.hpp"

class HTTPStatusLine
{
    public:
        static const std::string    m_protocol;
        const std::string           m_status_text;           
        HTTPStatusCode              m_status_code;

        HTTPStatusLine(HTTPStatusCode status_code);
};

std::ostream& operator<<(std::ostringstream&, const HTTPStatusLine& http_status_line);
#endif