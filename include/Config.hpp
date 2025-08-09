#pragma once
#ifndef CONFIG_HPP
# define CONFIG_HPP
# include "Pch.hpp" // IWYU pragma: keep
# include <string>
# include <unordered_map>
# include <vector>
# include <optional>
# include <filesystem>
# include <stack>
# include <algorithm>
# include "Http.hpp"

//-----------------------------------------------------------------------------
// Helper classes
//-----------------------------------------------------------------------------
class Ipv4Address
{
    public:
        uint32_t m_address;
        Ipv4Address(const std::string);
        Ipv4Address(int32_t);
        uint32_t from_string(const std::string&) const;
        const std::string to_string(void) const;
};

std::vector<std::string> split(const std::string to_split, const char delimiter);
bool operator==(const Ipv4Address&, const Ipv4Address&);

class Port
{
    public:
        const uint16_t m_port;
        uint16_t from_string(const std::string&) const;
        const std::string to_string(void) const;
        Port(const std::string&);
        Port(uint16_t);
};

bool operator==(const Port&, const Port&);

//-----------------------------------------------------------------------------
// Config file directives/keywords
//-----------------------------------------------------------------------------

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
        HTTPStatusCode enforce_error_code(HTTPStatusCode);
    public:
        ErrorPage(const std::string& status_code, const std::filesystem::path& path);
        const HTTPStatusCode m_status_code;
        const std::filesystem::path m_path;
};

bool operator==(const ErrorPage& a, const ErrorPage& b);

class Return: public ConfigDirective
{
    public:
        const HTTPStatusCode m_status_code;
        const std::string m_uri;
        Return(const std::string& status_code, const std::string uri);
        Return(HTTPStatusCode status_code, const std::string uri);
};

class AutoIndex: public ConfigDirective
{
    public:
        const bool m_on;
        AutoIndex(const std::string&);
};

class Index: public ConfigDirective
{
    public:
        const std::vector<std::string> m_files;
        Index(const std::vector<std::string>&);
};

class LimitExcept: public ConfigDirective
{
    private:
        const std::vector<HTTPMethod> from_string(const std::vector<std::string>&) const;
    public:
        const std::vector<HTTPMethod> m_allowed_methods;
        LimitExcept(const std::vector<std::string>&);
};

class UploadStore: public ConfigDirective
{
    public:
        const std::string m_path;
        UploadStore(const std::string&);
};

class LocationContext: public ConfigDirective
{
    public:
        LocationContext(const std::string&);
        const std::string m_uri;
        std::optional<Root> m_root;
        std::vector<ErrorPage> m_error_pages;
        std::optional<Return> m_return;
        std::optional<AutoIndex> m_auto_index;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<Index> m_index;
        std::optional<LimitExcept> m_limit_except;
        std::optional<UploadStore> m_upload_store;
};

bool operator==(const LocationContext&, const LocationContext&);

class ServerContext: public ConfigDirective
{
    public:
        ServerContext();
        LocationContext& get_location(void);
        bool is_valid(void) const;
        std::vector<LocationContext> m_location_contexts;
        std::vector<ErrorPage> m_error_pages;
        std::optional<Return> m_return;
        std::optional<ServerName> m_server_name;
        std::optional<Listen> m_listen;
        std::optional<Root> m_root;
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<AutoIndex> m_auto_index;
        std::optional<Index> m_index;
        std::optional<UploadStore> m_upload_store;
};

bool operator==(const ServerContext&, const ServerContext&);

class HttpContext
{
  public:
    std::vector<ErrorPage> m_error_pages;
    std::optional<Root> m_root;
    std::optional<ClientMaxBodySize> m_client_max_body_size;
    std::optional<AutoIndex> m_auto_index;
    std::optional<Index> m_index;
    std::optional<UploadStore> m_upload_store;
    HttpContext() = default;
};

//-----------------------------------------------------------------------------
// Config file lexer
//-----------------------------------------------------------------------------
enum class ContextName
{
    Http,
    Server,
    Location
};

enum class TokenType: int
{
    OpenBrace, // {
    CloseBrace, // }
    Semicolon, // ;
    Colon, // :
    Path,
	Uri,
    String,
    Number,
	IPv4,
	On,
	Off,

    // HTTP methods
    Get,
    Post,
    Delete,

    // NGINX keywords
    ClientMaxBodySize,
    ErrorPage,
    Http,
    Server,
    ServerName,
    Root,
    Location,
    Listen,
	Return,
	AutoIndex,
    Index,
    LimitExcept,
    UploadStore,

    Eof,
	TokenTypeCount
};


class Token
{
    public:
        const int m_linenum;
        TokenType m_token_type;
        std::string m_str;
        Token(TokenType, int);
        Token(TokenType, std::string, int);
		void print(void) const;
};

std::string stringify(TokenType type);

class Scanner
{
    private:
        size_t m_index;
		size_t m_linenum;
        const std::string m_in;
        const size_t    m_inlen;
        char advance(void);
        char peek(void) const;
        void skip_whitespace(void);
        std::vector<Token> m_tokens;
        void scan_token(void);
        void scan_string(void);
        void scan_number(void);
		bool scan_on_off(void);
        void add_token(TokenType);
        void add_token(TokenType, std::string);
        TokenType get_token_type(const std::string& str);
        bool at_end(void) const;
    public:
        Scanner(std::string);
        TokenType get_token(void);
        const std::vector<Token>& scan(void);
		void print_tokens(void) const;
};

//-----------------------------------------------------------------------------
// Config class
//-----------------------------------------------------------------------------
class Config
{
    public:
        HttpContext m_http_context;
        std::vector<ServerContext> m_servers;
        Config();
        ServerContext& get_server(void);
        void finalize(void);
};

//-----------------------------------------------------------------------------
// Config file parser
//-----------------------------------------------------------------------------
class Parser
{
    public:
        class                       Error;
        Parser(const std::vector<Token>& tokens, Config& config);
        void parse(void);

    private:
        const std::vector<Token>&   m_tokens;
        std::stack<ContextName>     m_contexts;
        void                        push_context(ContextName);
        void                        pop_context(void);
		Config&                     m_config;
        int                         m_current;
        bool                        at_end(void);
        bool                        at_end(void) const;
        bool                        match(const std::vector<TokenType>);
        bool                        check(TokenType);
        const Token&                advance(void);
        const Token&                previous(void) const;
        const Token&                consume(TokenType, const char *error);
        const Token&                consume(std::vector<TokenType>, const char *error);
        const Token&                peek(void) const;
        const Token&                next(void) const;
        void                        log_error(const std::string, const Token&) const;
        void                        log_error(const std::string) const;
        void                        parse_block(void);
        void                        parse_statement(void);
        bool                        is_valid_dir_path(const std::string) const;
        bool                        is_valid_file_path(const std::string) const;
        
        void                        parse_server_name(void);
        void                        parse_error_page(void);
        void                        parse_listen(void);
        void                        parse_location(void);
        void                        parse_root(void);
        void                        parse_client_max_body_size(void);
        void                        parse_server(void);
        void                        parse_return(void);
        void                        parse_autoindex(void);
        void                        parse_index(void);
        void                        parse_limit_except(void);
        void                        parse_upload_store(void);

        void                        set_server_name(ServerName);
        void                        set_error_page(ErrorPage);
        void                        set_listen(Listen);
        void                        set_location(LocationContext);
        void                        set_root(Root);
        void                        set_client_max_body_size(ClientMaxBodySize);
        void                        set_server(ServerContext);
        void                        set_return(Return);
        void                        set_auto_index(AutoIndex);
        void                        set_index(Index);
        void                        set_limit_except(LimitExcept);
        void                        set_upload_store(UploadStore);
};

class Parser::Error: public std::runtime_error
{
    public:
        Error();
};

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
void read_config_file(Config& config, const std::filesystem::path& path);
std::vector<ServerContext> get_server_config(const std::filesystem::path& path);
#endif