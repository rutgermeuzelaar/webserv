/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequestMethod.hpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/25 14:18:35 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/25 14:43:23 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_METHOD_HPP
# define HTTP_REQUEST_METHOD_HPP
# include <string>

class HTTPRequestMethod
{
    public:
        static const std::string _get;
        static const std::string _post;
        static const std::string _delete;

    public:
        static const std::string& GET(void); 
        static const std::string& POST(void);
        static const std::string& DELETE(void);
};
#endif