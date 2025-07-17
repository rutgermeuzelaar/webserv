#ifndef CGI_HPP
# define CGI_HPP
# include <string>
# include <filesystem>
# include <list>
# include <optional>
# include <chrono>
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
        std::list<CgiProcess> m_children;
        const std::chrono::milliseconds m_timeout_ms;
        const std::string get_script_name(const std::string& uri) const;
        void reap_dtor(void);
    public:
        Cgi(char **envp, size_t timeout_ms);
        ~Cgi(); // should reap child processes
        Cgi(const Cgi&) = delete;
        Cgi& operator=(const Cgi&) = delete;
        std::optional<std::pair<int, Response>> reap(void);
        void add_process(const Request& request, Epoll& epoll, int client_fd, const LocationContext* location, const ServerContext& config);
        void read_pipes(void);
        bool is_cgi_fd(int fd) const;
        void timeout(void);
};

std::optional<const std::string> find_binary(char *const *envp, const std::string& binary);
#endif