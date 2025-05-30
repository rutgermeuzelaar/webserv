/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigStatement.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:15:27 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/28 16:09:52 by rmeuzela      ########   odam.nl         */
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

static bool status_codes_overlap(const std::vector<HTTPStatusCode>& a, const std::vector<HTTPStatusCode>& b)
{
     for (const auto& it: a)
    {
        if (std::find(b.begin(), b.end(), it) != b.end())
        {
            return (true);
        }
    }
    return (false);
}

bool operator==(const ErrorPage& a, const ErrorPage& b)
{
    return (status_codes_overlap(a.m_status_codes, b.m_status_codes));
}

std::vector<HTTPStatusCode> Return::codes_from_string(const std::vector<std::string>& codes_str) const
{
    std::vector<HTTPStatusCode> codes;
    int int_rep;

    for (const auto& it: codes_str)
    {
        int_rep = std::stoi(it);
        if (!is_http_status_code(int_rep))
        {
            throw std::runtime_error("Not a HTTP status code.");
        }
        codes.push_back(static_cast<HTTPStatusCode>(int_rep));
    }
    return codes;
}

Return::Return(const std::vector<std::string>& codes_str, const std::string uri)
    : m_status_codes {codes_from_string(codes_str)}
    , m_uri {uri}
{
    
}

bool operator==(const Return& a, const Return& b)
{
    return (status_codes_overlap(a.m_status_codes, b.m_status_codes));
}

AutoIndex::AutoIndex(const std::string& status)
    : m_on {status == "on"}
{
   
}