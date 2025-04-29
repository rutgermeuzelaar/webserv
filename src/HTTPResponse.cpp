/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 21:59:45 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/29 18:52:11 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "defines.h"
#include "HTTPStatusCode.hpp"
#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse(HTTPStatusCode status_code)
    : m_status_line(status_code)
{
   
}

const std::string& HTTPResponse::to_str(void)
{
    std::ostringstream  stream;

    stream << m_status_line << LINE_BREAK;
    for (auto const&i: m_headers)
    {
        stream << i.first << ": " << i.second << LINE_BREAK;
    }
    stream << LINE_BREAK;
    stream << m_body;
    m_str = stream.str();
    return m_str;
}

bool HTTPResponse::set_body(const std::string html_path)
{
    std::ifstream       in(html_path);
    std::stringstream   buffer;

    if (in.fail())
    {
        return (false);
    }
    buffer << in.rdbuf();
    if (buffer.fail())
    {
        return (false);
    }
    m_body = buffer.str();
    return (true);
}
