/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfig.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/11 17:13:50 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP
# include <string>
# include <vector>
# include <map>

class ServerConfig
{
    private:
    
    public:
        std::string                             m_name;
        std::string                             m_address;
        int                                     m_port;
        std::map<std::vector<int>, std::string> m_error_page;
        std::map<std::string, std::string>      m_location;
        size_t                                  m_client_max_body_size;
};
#endif