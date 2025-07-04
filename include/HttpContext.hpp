#ifndef HTTP_CONTEXT_HPP
# define HTTP_CONTEXT_HPP
# include <vector>
# include "ConfigDirective.hpp"

class HttpContext
{
  public:
    std::vector<ErrorPage> m_error_pages;
    std::optional<Root> m_root;
    std::optional<ClientMaxBodySize> m_client_max_body_size;
    std::optional<AutoIndex> m_auto_index;
    HttpContext();
};
#endif