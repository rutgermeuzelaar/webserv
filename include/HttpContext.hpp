/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpContext.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:16:14 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 20:09:23 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONTEXT_HPP
# define HTTP_CONTEXT_HPP
# include <vector>
# include "ServerContext.hpp"
# include "ConfigCollection.hpp"

class HttpContext
{
  public:
    ConfigCollection<ServerContext> m_servers;
    ConfigCollection<ErrorPage> m_error_pages;
    HttpContext();
    void validate(void) const;
};
#endif