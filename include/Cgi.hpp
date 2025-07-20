#ifndef CGI_HPP
# define CGI_HPP
# include <string>
# include <filesystem>
# include <list>
# include <optional>
# include <memory>
# include <sys/types.h>
# include "LocationContext.hpp"

class Epoll;
class Request;
class CgiProcess;
class Client;

class Cgi
{
    private:
        char **m_envp;
        std::vector<std::shared_ptr<CgiProcess>> m_children;
        const std::chrono::milliseconds m_timeout_ms;
        const std::string get_script_name(const std::string& uri) const;
        void reap_dtor(void);
    public:
        Cgi(char **envp, size_t timeout_ms);
        ~Cgi(); // should reap child processes
        Cgi(const Cgi&) = delete;
        Cgi& operator=(const Cgi&) = delete;

        void reap(void);
        void add_process(Client& client, const Request& request, Epoll& epoll, const LocationContext* location, const ServerContext& config, Server& server);
        bool is_cgi_fd(int fd) const;
        void timeout(void);
        bool has_children(void) const;
        CgiProcess& get_child(int fd);
        void erase_child(int fd);
        std::list<CgiProcess>& get_children();
};

std::optional<const std::string> find_binary(char *const *envp, const std::string& binary);
#endif