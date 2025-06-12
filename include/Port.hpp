/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Port.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/26 18:38:52 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 18:55:09 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORT_HPP
# define PORT_HPP
# include <inttypes.h>
# include <string>

class Port
{
    public:
        const uint16_t m_port;
        uint16_t from_string(const std::string&) const;
        const std::string to_string(void) const;
        Port(const std::string&);
        Port(uint16_t);
};

bool operator==(const Port&, const Port&);
#endif