/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationContext.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:40:28 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 14:34:43 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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
