/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerContext.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/28 16:21:34 by rmeuzela      ########   odam.nl         */
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
# include "ConfigCollection.hpp"

class ServerContext
{
    public:
        ServerContext();
        LocationContext& get_location(void);
        ConfigCollection<LocationContext> m_location_contexts;
        ConfigCollection<ErrorPage> m_error_pages;
        ConfigCollection<Return> m_returns;
        std::optional<ServerName> m_server_name;
        std::optional<Listen> m_listen;
        std::optional<Root> m_root;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<AutoIndex> m_auto_index;
		// bool validate() const;
};

#endif