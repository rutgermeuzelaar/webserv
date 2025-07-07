#ifndef CGI_HPP
# define CGI_HPP
# include <string>
# include <filesystem>
# include <vector>
# include <optional>
# include <chrono>
# include <sys/types.h>
# include "LocationContext.hpp"

class Epoll;
class Request;
class Response;
class ServerContext;

class CgiProcess
{
    public:
        CgiProcess(int read_fd, int client_fd, pid_t pid, const LocationContext* location, const ServerContext& config);
        CgiProcess& operator=(const CgiProcess&);
        int m_read_fd;
        int m_client_fd;
        std::chrono::_V2::steady_clock::time_point m_start;
        pid_t m_pid;
        std::string m_buffer;
        const LocationContext* m_location;
        const ServerContext& m_config;
};

class Cgi
{
    private:
        char **m_envp;
        std::vector<CgiProcess> m_children;
        const std::chrono::milliseconds m_timeout_ms;
        const std::string get_script_name(const std::string& uri) const;
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