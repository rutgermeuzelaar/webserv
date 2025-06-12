/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationContext.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/22 16:36:22 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 14:33:00 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_CONTEXT_HPP
# define LOCATION_CONTEXT_HPP
# include <string>
# include "ConfigDirective.hpp"

class ConfigDirective;
class LocationContext: public ConfigDirective
{
    public:
        LocationContext(const std::string&);
        const std::string m_uri;
        std::optional<Root> m_root;
        std::vector<ErrorPage> m_error_pages;
        std::vector<Return> m_returns;
        std::optional<AutoIndex> m_auto_index;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
};

bool operator==(const LocationContext&, const LocationContext&);
#endif