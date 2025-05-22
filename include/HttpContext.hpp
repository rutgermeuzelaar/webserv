/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpContext.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:16:14 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/22 16:36:15 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CONTEXT_HPP
# define HTTP_CONTEXT_HPP
# include <vector>
# include "ServerContext.hpp"
class HttpContext
{
    private:
		size_t						m_server_index;
    std::vector <ServerContext>	m_servers;
    
    public:
		HttpContext();
        void validate(void) const;
};

#endif