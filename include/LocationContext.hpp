/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   LocationContext.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/22 16:36:22 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/28 16:21:17 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_CONTEXT_HPP
# define LOCATION_CONTEXT_HPP
# include <string>
# include "ConfigStatement.hpp"
# include "ConfigCollection.hpp"

class LocationContext
{
    public:
        const std::string m_uri;
        LocationContext(const std::string&); 
        std::optional<Root> m_root;
        ConfigCollection<ErrorPage> m_error_pages;
        ConfigCollection<Return> m_returns;
        std::optional<AutoIndex> m_auto_index;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
};

bool operator==(const LocationContext&, const LocationContext&);
#endif