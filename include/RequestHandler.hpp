/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:13:45 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/05 18:02:21 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP
# include <optional>
# include "Response.hpp"
# include "Request.hpp"
# include "Config.hpp"

class RequestHandler
{
	private:
		const Config& m_config;
	
	public:
		RequestHandler(const Config&);
		Response handle_get(const Request& request);
		void handle_post(const Request& request);
		void handle_delete(const Request& request);
		std::optional<std::string> map_uri(std::string uri);
		void handle(const Request& request, int fd);
};
#endif