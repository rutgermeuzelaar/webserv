/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigStatement.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:13:09 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 12:13:43 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_STATEMENT_HPP
# define CONFIG_STATEMENT_HPP
# include <filesystem>
# include <string>
# include <optional>

class Root
{
    public:
        std::filesystem::path m_path;
        Root();
};

class ClientMaxBodySize
{
    public:
        size_t    m_size;
        ClientMaxBodySize();
};

class Location
{
    public:
        std::string m_uri;
        std::optional<Root> m_root;
        Location();
};

class ServerName
{
    private:
        std::optional<std::string> m_name;
    
    public:
        ServerName();
        void set_name(std::string);
};

class Listen
{
	public:
		const int32_t m_ipv4;
		const int32_t m_port;
		Listen();
		Listen(const std::string ipv4);
		Listen(const std::string ipv4, const std::string port);
};
#endif