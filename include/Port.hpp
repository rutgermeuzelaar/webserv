/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Port.hpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/26 18:38:52 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/26 18:41:30 by rmeuzela      ########   odam.nl         */
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
        Port(const std::string&);
        Port(uint16_t);
};
#endif