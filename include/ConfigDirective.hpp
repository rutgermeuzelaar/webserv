/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigDirective.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/07 07:43:50 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 13:10:09 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_DIRECTIVE_HPP
# define CONFIG_DIRECTIVE_HPP
# include <vector>
# include <filesystem>
# include <optional>
# include <algorithm>
# include "Ipv4Address.hpp"
# include "Port.hpp"
# include "ContextName.hpp"
# include "HTTPStatusCode.hpp"

class ConfigDirective
{
    private:
        const bool  m_is_unique; // it's only allowed to be set once in a given context
    public:
        ConfigDirective(bool);
        bool is_unique(void) const;

};

template <typename T> void add_to_context(std::optional<T>& opt_ref, T directive)
{
    if (directive.is_unique() && opt_ref.has_value())
    {
        throw std::runtime_error("Value must be unique in the current context.");
    }
    opt_ref.emplace(directive);
}

template <typename T> void add_to_context(std::vector<T>& collection, T directive)
{
    if (directive.is_unique() && \
    std::find(collection.begin(), collection.end(), directive) != collection.end())
    {
        throw std::runtime_error("Value must be unique in the current context.");
    }
    collection.push_back(directive);
}

class Root: public ConfigDirective
{
    public:
        const std::filesystem::path m_path;
        Root(std::filesystem::path);
};

class ClientMaxBodySize: public ConfigDirective
{
    public:
        const size_t    m_size;
        ClientMaxBodySize(size_t);
        ClientMaxBodySize(const std::string&);
        size_t from_string(const std::string&) const;
};

class ServerName: public ConfigDirective
{
    public:
        const std::string m_name;
        ServerName(std::string);
};

class Listen: public ConfigDirective
{
	public:
		std::optional<Ipv4Address> m_ipv4;
		std::optional<Port> m_port;
		Listen();
		Listen(const std::string ipv4);
		Listen(const std::string ipv4, const std::string port);
};

bool operator==(const Listen&, const Listen&);

class ErrorPage: public ConfigDirective
{
    private:
        std::vector<HTTPStatusCode> create_status_codes(const std::vector<std::string>&);
    public:
        ErrorPage(const std::vector<std::string>&, const std::filesystem::path&);
        const std::vector<HTTPStatusCode> m_status_codes;
        const std::filesystem::path m_path;
};

bool operator==(const ErrorPage&, const ErrorPage&);

class Return: public ConfigDirective
{
    public:
        const HTTPStatusCode m_status_code;
        const std::string m_uri;
        Return(const std::string& status_code, const std::string uri);
};

class AutoIndex: public ConfigDirective
{
    public:
        const bool m_on;
        AutoIndex(const std::string&);
};
#endif