/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Ipv4Address.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/25 16:47:25 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 14:45:03 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "Ipv4Address.hpp"
#include "Utilities.hpp"

static uint8_t octet_from_string(const std::string& octet)
{
    int32_t stoi_ret;

    stoi_ret = std::stoi(octet);
    if (stoi_ret < std::numeric_limits<uint8_t>::min() || stoi_ret > std::numeric_limits<uint8_t>::max())
    {
        throw std::runtime_error("Octet out of range.");
    }
    return (stoi_ret);
}

uint32_t Ipv4Address::from_string(const std::string& str) const
{
    const std::vector<std::string>& octets = split(str, '.');

    if (octets.size() != 4)
    {
        throw std::runtime_error("IPv4 address should have 4 octets.");
    }
    return (octet_from_string(octets[0]) << 24 | octet_from_string(octets[1]) << 16 \
    | octet_from_string(octets[2]) << 8 | octet_from_string(octets[3]));
}

Ipv4Address::Ipv4Address(const std::string ipv4_str)
    : m_address {from_string(ipv4_str)}
{
        
}

bool operator==(const Ipv4Address& a, const Ipv4Address& b)
{
    return a.m_address == b.m_address;
}
