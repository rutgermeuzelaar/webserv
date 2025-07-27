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
        void reap_dtor(void);
        void add_process_default(Client& client, const Request& request, Epoll& epoll, const LocationContext* location, const ServerContext& config, Server& server);
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
        void erase_child(pid_t pid, bool require_connection);
};

#endif