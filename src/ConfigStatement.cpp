/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigStatement.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:15:27 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/27 19:09:39 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <string>
#include <algorithm>
#include <limits>
#include "ConfigStatement.hpp"
#include "HTTPStatusCode.hpp"

Listen::Listen()
{
    
}

Listen::Listen(const std::string ipv4_str)
    : m_ipv4 {Ipv4Address(ipv4_str)}
{
    
}

Listen::Listen(const std::string ipv4_str, const std::string port_str)
    : m_ipv4 {Ipv4Address(ipv4_str)}
    , m_port {Port(port_str)}
{
    
}

ServerName::ServerName(std::string name)
    : m_name {name}
{
    
}

Root::Root(std::filesystem::path path)
    : m_path {path}
{
    
}

size_t ClientMaxBodySize::from_string(const std::string& size) const
{
    long body_size;
    
    body_size = std::stol(size);
    if (body_size < 0|| static_cast<size_t>(body_size) > std::numeric_limits<size_t>::max())
    {
        throw std::runtime_error("Invalid client max body size.");
    }
    return body_size;
}

ClientMaxBodySize::ClientMaxBodySize(size_t size)
    : m_size {size}
{
    
}

ClientMaxBodySize::ClientMaxBodySize(const std::string& size)
    : m_size {from_string(size)}
{
    
}

std::vector<HTTPStatusCode> ErrorPage::create_status_codes(const std::vector<std::string>& codes)
{
    std::vector<HTTPStatusCode> converted;
    int int_rep;

    for (const std::string& it: codes)
    {
        int_rep = std::stoi(it);
        if (!is_http_status_code(int_rep))
        {
            throw std::runtime_error("Invalid HTTP code.");
        }
        if (int_rep < 400 || int_rep > 599)
        {
            throw std::runtime_error("Not a HTTP error code.");
        }
        converted.push_back(static_cast<HTTPStatusCode>(int_rep));
    }
    return converted;
}

ErrorPage::ErrorPage(const std::vector<std::string>& codes, const std::filesystem::path& path)
    : m_status_codes {create_status_codes(codes)}
    , m_path {path}
{
    
}

bool operator==(const ErrorPage& a, const ErrorPage& b)
{
    for (const HTTPStatusCode& it: a.m_status_codes)
    {
        if (std::find(b.m_status_codes.begin(), b.m_status_codes.end(), it) != b.m_status_codes.end())
        {
            return (true);
        }
    }
    return (false);
}
