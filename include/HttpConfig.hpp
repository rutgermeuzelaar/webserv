/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpConfig.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:16:14 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/11 14:18:24 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONFIG_HPP
# define HTTP_CONFIG_HPP
# include <vector>
# include "ServerConfig.hpp"
class HttpConfig
{
    private:
        std::vector <ServerConfig> m_servers;
    
    public:
        void validate(void) const;
};

#endif