/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatusLine.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 21:56:08 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/28 22:39:18 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>
#include <string>
#include "defines.h"
#include "HTTPStatusLine.hpp"
#include "HTTPStatusCode.hpp"

const std::string HTTPStatusLine::m_protocol = PROTOCOL;

HTTPStatusLine::HTTPStatusLine(HTTPStatusCode status_code)
    : m_status_text {get_http_status_text(status_code)}
    , m_status_code {status_code}
{
    
}

std::ostream& operator<<(std::ostringstream& ostream, const HTTPStatusLine& http_status_line)
{
    ostream << http_status_line.m_protocol << ' ' << http_status_line.m_status_code << ' ' << http_status_line.m_status_text;
    return ostream;
}
