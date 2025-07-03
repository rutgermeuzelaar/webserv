/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpContext.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:16:14 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 17:43:02 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONTEXT_HPP
# define HTTP_CONTEXT_HPP
# include <vector>
# include "ConfigDirective.hpp"

class HttpContext
{
  public:
    std::vector<ErrorPage> m_error_pages;
    std::optional<Root> m_root;
    std::optional<ClientMaxBodySize> m_client_max_body_size;
    std::optional<AutoIndex> m_auto_index;
    std::optional<Index> m_index;
    HttpContext();
};
#endif