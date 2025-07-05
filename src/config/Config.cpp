#include "Pch.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "Config.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "Utilities.hpp"

Config::Config()
{
    
}

void Config::finalize(void)
{
    if (!is_unique<ServerContext>(m_servers))
    {
        throw std::runtime_error("Servers contain overlap, please check the configuration file.");
    }
    for (auto& i: m_servers)
    {
        merge_config(m_http_context, i); 
    }
}

ServerContext& Config::get_server()
{
	return (m_servers.back());
}

void read_config_file(Config& config, const std::filesystem::path& path)
{
    std::ifstream config_file(path);
    std::stringstream buffer;

    if (config_file.fail())
    {
		throw std::runtime_error("An error occured when opening the supplied"
		" configuration file.");
    }
    buffer << config_file.rdbuf();
    Scanner scanner(buffer.str());
    const std::vector<Token>& tokens = scanner.scan();
    Parser parser(tokens, config);
    parser.parse();
}

void merge_config(const HttpContext& http, ServerContext& server)
{
    merge_directive<ErrorPage>(http.m_error_pages, server.m_error_pages);
    merge_directive<Root>(http.m_root, server.m_root);
    merge_directive<ClientMaxBodySize>(http.m_client_max_body_size, server.m_client_max_body_size);
    merge_directive<AutoIndex>(http.m_auto_index, server.m_auto_index);
    merge_directive<Index>(http.m_index, server.m_index);
}

void merge_config(const ServerContext& merge_from, ServerContext& merge_into)
{
    merge_directive<LocationContext>(merge_from.m_location_contexts, merge_into.m_location_contexts);
    merge_directive<ErrorPage>(merge_from.m_error_pages, merge_into.m_error_pages);
    merge_directive<Return>(merge_from.m_return, merge_into.m_return);
    merge_directive<ServerName>(merge_from.m_server_name, merge_into.m_server_name);
    merge_directive<Listen>(merge_from.m_listen, merge_into.m_listen);
    merge_directive<Root>(merge_from.m_root, merge_into.m_root);
    merge_directive<ClientMaxBodySize>(merge_from.m_client_max_body_size, merge_into.m_client_max_body_size);
    merge_directive<AutoIndex>(merge_from.m_auto_index, merge_into.m_auto_index);
    merge_directive<Index>(merge_from.m_index, merge_into.m_index);
}

void load_defaults(const std::filesystem::path& path, std::vector<ServerContext>& servers)
{
    Config config;

    read_config_file(config, path);
    ServerContext def = config.m_servers.at(0);
    for (auto& it: servers)
    {
        merge_config(def, it);
    }
}

std::vector<ServerContext> get_server_config(const std::filesystem::path& path)
{
    Config config;

    read_config_file(config, path);
    config.finalize();
    if (std::filesystem::exists(DEFAULT_CONF))
    {
        load_defaults(DEFAULT_CONF, config.m_servers);
        for (size_t i = 0; i < config.m_servers.size(); ++i)
        {
            merge_config(config.m_http_context, config.m_servers[i]);
            if (!config.m_servers[i].is_valid())
            {
                std::cerr << "Unvalid config for server config: " << i + 1 << '\n';
                throw std::runtime_error("Invalid configuration file");
            }
        }
        return config.m_servers;
    }
    throw std::runtime_error("no such file or directory: " DEFAULT_CONF);
}