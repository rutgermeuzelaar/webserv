/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerContext.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 11:55:30 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 13:02:03 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <algorithm>
#include "ServerContext.hpp"

ServerContext::ServerContext()
{
        
}

void ServerContext::add_location(std::string uri)
{
    LocationContext location_context(uri);

    auto it = std::find(m_location_contexts.begin(), m_location_contexts.end(), location_context);
    if (it != m_location_contexts.end())
    {
        throw std::runtime_error("Cannot redefine location, URI already in use.");
    }
    m_location_contexts.push_back(location_context);
}

LocationContext& ServerContext::get_location(void)
{
    if (m_location_contexts.size() == 0)
    {
        throw std::runtime_error("Missing location block.");
    }
    return (m_location_contexts.back());
}
