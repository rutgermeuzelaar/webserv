/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigStatement.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:15:27 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 12:18:01 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include "ConfigStatement.hpp"

ServerName::ServerName()
{
    
}

void ServerName::set_name(std::string name)
{
    if (m_name.has_value())
    {
        throw std::runtime_error("server_name already set.");
    }
    m_name.emplace(name);
}
