/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Ipv4Address.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/25 16:47:25 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/26 20:17:27 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "Ipv4Address.hpp"

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

std::vector<std::string> split(const std::string to_split, const char delimiter)
{
    const size_t length = to_split.length();
    std::vector<std::string> split_str;
    size_t                      i;
    size_t                      j;
    
    i = 0;
    j = 0;
    while (i < length)
    {
        if (to_split.at(i) == delimiter)
        {
            split_str.push_back(to_split.substr(j, i - j));
            j = i + 1;            
        }
        i++;
    }
    split_str.push_back(to_split.substr(j, length - j));
    return split_str;
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