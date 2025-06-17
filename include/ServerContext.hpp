/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerContext.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 13:14:26 by rmeuzela      ########   odam.nl         */
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
# include "ConfigDirective.hpp"
# include "LocationContext.hpp"

class ConfigDirective;
class ServerContext: public ConfigDirective
{
    public:
        ServerContext();
        LocationContext& get_location(void);
        std::vector<LocationContext> m_location_contexts;
        std::vector<ErrorPage> m_error_pages;
        std::optional<Return> m_return;
        std::optional<ServerName> m_server_name;
        std::optional<Listen> m_listen;
        std::optional<Root> m_root;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<AutoIndex> m_auto_index;
};

bool operator==(const ServerContext&, const ServerContext&);
#endif