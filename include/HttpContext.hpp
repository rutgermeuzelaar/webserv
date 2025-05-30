/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpContext.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:16:14 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/28 16:21:04 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONTEXT_HPP
# define HTTP_CONTEXT_HPP
# include <vector>
# include "ServerContext.hpp"
# include "ConfigStatement.hpp"
# include "ConfigCollection.hpp"

class HttpContext
{
  public:
    ConfigCollection<ServerContext> m_servers;
    ConfigCollection<ErrorPage> m_error_pages;
    std::optional<Root> m_root;
    std::optional<ClientMaxBodySize> m_client_max_body_size;
    std::optional<AutoIndex> m_auto_index;
    HttpContext();
    void validate(void) const;
};
#endif