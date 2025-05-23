/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerContext.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 12:34:01 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONTEXT_HPP
# define SERVER_CONTEXT_HPP
# include <string>
# include <vector>
# include <map>
# include <filesystem>
# include <optional>
# include "Lexer.hpp"
# include "ConfigStatement.hpp"
# include "LocationContext.hpp"

class ServerContext
{
    private:
        std::vector<LocationContext> m_location_contexts;
    public:
        ServerContext();
        ServerName m_server_name;
        void add_location(std::string);
        LocationContext& get_location(void);
        // std::optional<ClientMaxBodySize> m_client_max_body_size;
        // std::optional<ServerName> m_server_name;
		// Listen m_listen;
		// bool validate() const;
};

#endif