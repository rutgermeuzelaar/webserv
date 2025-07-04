#include <stdexcept>
#include <string>
#include <algorithm>
#include <limits>
#include <cassert>
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

HTTPStatusCode ErrorPage::enforce_error_code(HTTPStatusCode status_code)
{
    const int int_rep = static_cast<int>(status_code);

    if (int_rep < 400 || int_rep > 599)
    {
        throw std::runtime_error("Not a HTTP error code.");
    }
    return status_code;
}

ErrorPage::ErrorPage(const std::string& status_code, const std::filesystem::path& path)
    : ConfigDirective(true)
    , m_status_code {enforce_error_code(from_string(status_code))}
    , m_path {path}
{
    
}

bool operator==(const ErrorPage& a, const ErrorPage& b)
{
    return (a.m_status_code == b.m_status_code);
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

Index::Index(const std::vector<std::string>& files)
    : ConfigDirective(true)
    , m_files {files}
{

}

const std::vector<HTTPMethod> LimitExcept::from_string(const std::vector<std::string>& methods_str) const
{
    std::vector<HTTPMethod> methods;

    for (auto str: methods_str)
    {
        // use new branch function later
        if (str == "GET") methods.push_back(HTTPMethod::GET);
        else if (str == "POST") methods.push_back(HTTPMethod::POST);
        else if (str == "DELETE") methods.push_back(HTTPMethod::DELETE);
        else assert("HTTP methods should be sanitized before this" && false); 
    }
    return methods;
}

LimitExcept::LimitExcept(const std::vector<std::string>& allowed_methods)
    : ConfigDirective(true)
    , m_allowed_methods {from_string(allowed_methods)}
{

}

UploadStore::UploadStore(const std::string& path)
    : ConfigDirective(true)
    , m_path {path}
{

}