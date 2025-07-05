#ifndef SERVER_CONTEXT_HPP
# define SERVER_CONTEXT_HPP
# include <string>
# include <vector>
# include <map>
# include <filesystem>
# include <optional>
# include "Lexer.hpp"
# include "ConfigDirective.hpp"
# include "LocationContext.hpp"

class ConfigDirective;
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
#endif