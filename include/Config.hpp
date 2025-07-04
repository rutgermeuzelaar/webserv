#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <string>
# include <unordered_map>
# include <vector>
# include <optional>
# include <filesystem>
# include "Lexer.hpp"
# include "ServerContext.hpp"
# include "HttpContext.hpp"
# include "ConfigDirective.hpp"

const std::unordered_map<std::string, TokenType> keywords {
    {"client_max_body_size", TokenType::ClientMaxBodySize},
    {"error_page", TokenType::ErrorPage},
    {"http", TokenType::Http},
    {"server", TokenType::Server},
    {"server_name", TokenType::ServerName},
    {"root", TokenType::Root},
    {"location", TokenType::Location},
    {"listen", TokenType::Listen},
    {"return", TokenType::Return},
    {"autoindex", TokenType::AutoIndex},
    {"on", TokenType::On},
    {"off", TokenType::Off},
    {"index", TokenType::Index},
    {"GET", TokenType::Get},
    {"POST", TokenType::Post},
    {"DELETE", TokenType::Delete},
    {"limit_except", TokenType::LimitExcept},
    {"upload_store", TokenType::UploadStore}
};

class Config
{
    public:
        HttpContext m_http_context;
        std::vector<ServerContext> m_servers;
        Config();
        ServerContext& get_server(void);
        void finalize(void);
};

template <typename T> void merge_directive(const std::optional<T>& from, std::optional<T>& to)
{
    if (from.has_value() && !to.has_value())
    {
        to.emplace(from.value());
    }
}

template <typename T> void merge_directive(const std::vector<T>& from, std::vector<T>& to)
{
    for (auto const& it: from)
    {
        if (std::find(to.begin(), to.end(), it) == to.end())
		{
            to.push_back(it);
        }
    }
}

void merge_config(const HttpContext& http, ServerContext& server);
void merge_config(const ServerContext& merge_from, ServerContext& merge_into);
void load_defaults(const std::filesystem::path& path, std::vector<ServerContext>& servers);
void read_config_file(Config& config, const std::filesystem::path& path);
std::vector<ServerContext> get_server_config(const std::filesystem::path& path);
#endif