/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigDirective.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:15:27 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 13:11:13 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <string>
#include <algorithm>
#include <limits>
#include "ConfigDirective.hpp"
#include "HTTPStatusCode.hpp"

ConfigDirective::ConfigDirective(bool is_unique)
    : m_is_unique {is_unique}
{
    
}

bool ConfigDirective::is_unique() const
{
    return (m_is_unique);
}

Listen::Listen()
    : ConfigDirective(true)
{
    
}

Listen::Listen(const std::string ipv4_str)
    : ConfigDirective(true)
    , m_ipv4 {Ipv4Address(ipv4_str)}
{
    
}

Listen::Listen(const std::string ipv4_str, const std::string port_str)
    : ConfigDirective(true)
    , m_ipv4 {Ipv4Address(ipv4_str)}
    , m_port {Port(port_str)}
{
    
}

bool operator==(const Listen& a, const Listen& b)
{
    if (a.m_ipv4.has_value() && a.m_port.has_value() && b.m_ipv4.has_value() && b.m_port.has_value())
    {
        if (a.m_ipv4.value() == b.m_ipv4.value() && \
        a.m_port.value() == b.m_port.value())
        {
            return (true);
        }
    }
    return (false);
}
ServerName::ServerName(std::string name)
    : ConfigDirective(true)
    , m_name {name}
{
    
}

Root::Root(std::filesystem::path path)
    : ConfigDirective(false)
    , m_path {path}
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
    : ConfigDirective(true)
    , m_size {size}
{
    
}

ClientMaxBodySize::ClientMaxBodySize(const std::string& size)
    : ConfigDirective(true)
    , m_size {from_string(size)}
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
    : ConfigDirective(true)
    , m_status_codes {create_status_codes(codes)}
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

Return::Return(const std::string& status_code, const std::string uri)
    : ConfigDirective(true)
    , m_status_code {from_string(status_code)}
    , m_uri {uri}
{
    
}

AutoIndex::AutoIndex(const std::string& status)
    : ConfigDirective(true)
    , m_on {status == "on"}
{
   
}
