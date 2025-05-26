/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerContext.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 11:55:30 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 19:04:22 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <algorithm>
#include "ServerContext.hpp"

ServerContext::ServerContext()
{
        
}

LocationContext& ServerContext::get_location()
{
    return (m_location_contexts.m_vector.back());
}
