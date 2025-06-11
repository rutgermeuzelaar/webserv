/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Port.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/26 18:41:35 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 14:46:08 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <limits>
#include <stdexcept>
#include "Port.hpp"

uint16_t Port::from_string(const std::string& port_str) const
{
    int32_t stoi_ret;

    stoi_ret = std::stoi(port_str);
    if (stoi_ret < std::numeric_limits<uint16_t>::min() || stoi_ret > std::numeric_limits<uint16_t>::max())
    {
        throw std::runtime_error("Port out of range.");
    }
    if (stoi_ret <= 1023)
    {
        throw std::runtime_error("Cannot assign port in system ports range (0 - 1023).");
    }
    if (stoi_ret > 65535)
    {
        throw std::runtime_error("Port number too high, must be less than or equal to 65535.");
    }
    return stoi_ret;
}

Port::Port(const std::string& port_str)
    : m_port {from_string(port_str)}
{
    
}

bool operator==(const Port& a, const Port& b)
{
    return a.m_port == b.m_port;
}
