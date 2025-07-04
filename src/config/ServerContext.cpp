#include <stdexcept>
#include <algorithm>
#include "ServerContext.hpp"

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