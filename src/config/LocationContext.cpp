#include "Pch.hpp"
#include "LocationContext.hpp"

LocationContext::LocationContext(const std::string& uri)
    : ConfigDirective(true)
    , m_uri {uri}
{
    
}

bool operator==(const LocationContext& a, const LocationContext& b)
{
    return (a.m_uri == b.m_uri);    
}
