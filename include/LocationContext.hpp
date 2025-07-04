#ifndef LOCATION_CONTEXT_HPP
# define LOCATION_CONTEXT_HPP
# include <string>
# include "ConfigDirective.hpp"

class ConfigDirective;
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
#endif