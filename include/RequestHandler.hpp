/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:13:45 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 19:26:36 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP
# include <optional>
# include "Response.hpp"
# include "Request.hpp"
# include "ServerContext.hpp"

class RequestHandler
{
	private:
		const ServerContext& m_config;
		Response handle_get(const Request& request);
		Response handle_post(const Request& request);
		Response handle_delete(const Request& request);

	public:
		RequestHandler(const ServerContext&);
		std::optional<std::string> map_uri(std::string uri);
		Response handle(const Request& request);
};
#endif