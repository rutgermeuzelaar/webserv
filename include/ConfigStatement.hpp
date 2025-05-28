/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigStatement.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 12:13:09 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/28 16:08:41 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_STATEMENT_HPP
# define CONFIG_STATEMENT_HPP
# include <filesystem>
# include <string>
# include <optional>
# include <vector>
# include "HTTPStatusCode.hpp"
# include "Ipv4Address.hpp"
# include "Port.hpp"

class Root
{
    public:
        const std::filesystem::path m_path;
        Root(std::filesystem::path);
};

class ClientMaxBodySize
{
    public:
        const size_t    m_size;
        ClientMaxBodySize(size_t);
        ClientMaxBodySize(const std::string&);
        size_t from_string(const std::string&) const;
};

class ServerName
{
    private:
        const std::string m_name;
    
    public:
        ServerName(std::string);
};

class Listen
{
	public:
		std::optional<Ipv4Address> m_ipv4;
		std::optional<Port> m_port;
		Listen();
		Listen(const std::string ipv4);
		Listen(const std::string ipv4, const std::string port);
};

class ErrorPage
{
    private:
        std::vector<HTTPStatusCode> create_status_codes(const std::vector<std::string>&);
    public:
        ErrorPage(const std::vector<std::string>&, const std::filesystem::path&);
        const std::vector<HTTPStatusCode> m_status_codes;
        const std::filesystem::path m_path;
};

bool operator==(const ErrorPage&, const ErrorPage&);

class Return
{
    private:
        std::vector<HTTPStatusCode> codes_from_string(const std::vector<std::string>&) const;
    public:
        const std::vector<HTTPStatusCode> m_status_codes;
        const std::string m_uri;
        Return(const std::vector<std::string>&, const std::string);
};

bool operator==(const Return&, const Return&);

class AutoIndex
{
    public:
        const bool m_on;
        AutoIndex(const std::string&);
};

template <typename T>
void set_statement_unique(std::optional<T>&member, T object)
{
    if (member.has_value())
    {
        throw std::runtime_error("Property already set.");
    }
    member.emplace(object);
};
template <typename T>
void set_statement(std::optional<T>&member, T object)
{
    member.emplace(object);
};
#endif