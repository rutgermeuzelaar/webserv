#include "Pch.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "Config.hpp"

ServerContext::ServerContext()
    : ConfigDirective(true)
{
        
}

LocationContext& ServerContext::get_location()
{
    return (m_location_contexts.back());
}

bool operator==(const ServerContext& a, const ServerContext& b)
{
    if (a.m_server_name.has_value() && b.m_server_name.has_value())
    {
        if (a.m_server_name.value().m_name == b.m_server_name.value().m_name)
        {
            return (true);
        }
    }
    if (a.m_listen.has_value() && b.m_listen.has_value())
    {
        if (a.m_listen.value() == b.m_listen.value())
        {
            return (true);
        }
    }
    return (false);
}

static void print_missing_directive(const std::string& directive)
{
    std::cerr << "Missing " << directive << " directive\n";
}

bool ServerContext::is_valid() const
{
    bool valid = true;

    if (!m_client_max_body_size.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::ClientMaxBodySize));
    }
    if (!m_server_name.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::ServerName));
    }
    if (!m_root.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::Root));
    }
    if (!m_listen.has_value() || !m_listen.value().m_ipv4.has_value() || !m_listen.value().m_port.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::Listen));
    }
    if (!m_auto_index.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::AutoIndex));
    }
    if (!m_index.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::Index));
    }
    if (!m_upload_store.has_value())
    {
        valid = false;
        print_missing_directive(stringify(TokenType::UploadStore));
    }
    return valid; 
}
