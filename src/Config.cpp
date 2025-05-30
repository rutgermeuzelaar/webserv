/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 11:38:40 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 19:01:30 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include "Config.hpp"

Config::Config()
{
      
}

ServerContext& Config::get_server()
{
    return (m_http_context.m_servers.m_vector.back());
}
