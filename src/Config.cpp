/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 11:38:40 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 12:26:02 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include "Config.hpp"

Config::Config()
{
      
}

ServerContext& Config::get_server()
{
    if (m_server_contexts.size() == 0)
    {
        throw std::runtime_error("Missing server block.");
    }
    return m_server_contexts.back();
}

void Config::add_server(void)
{
    m_server_contexts.push_back(ServerContext()); 
}
