#ifndef CGI_HPP
# define CGI_HPP
# include <string>
# include <filesystem>
# include <list>
# include <optional>
# include <memory>
# include <sys/types.h>
# include "Config.hpp"

class Epoll;
class Server;
enum class CgiProcessEvent;
class Client;

//-----------------------------------------------------------------------------
// Class to store requested CGI processes
//-----------------------------------------------------------------------------
class CgiProcess
{
    private:
        Server& m_server;
        bool m_client_connected;
        bool m_reaped;
        int m_fd;
        bool m_is_post;
        bool m_in_notify;
        HttpBody& m_http_body; // needed for POST
        bool m_reading_complete;
        bool m_writing_complete;

        void notify_observer(CgiProcessEvent);
        void check_state(void);
        
        void set_fd(int fd);
    public:
        CgiProcess(int fd, int client_fd, pid_t pid, const LocationContext* location, \
            const ServerContext& config, Server& server, HttpBody& http_body);
        CgiProcess& operator=(const CgiProcess&);
        ~CgiProcess();

        int m_client_fd;
        std::chrono::_V2::steady_clock::time_point m_start;
        pid_t m_pid;
        std::string m_buffer;
        const LocationContext* m_location;
        const ServerContext& m_config;
        int m_exit_status;

        void set_client_connected(bool status);
        void set_reaped(bool status);
        void set_is_post(bool status);
        void set_reading_complete(bool status);
        void set_writing_complete(bool status);
 
        bool get_client_connected(void) const;
        bool get_reaped(void) const;
        int  get_fd(void) const;

        void close_fd(Epoll& epoll);
        void read_fd(Epoll& epoll);
        void write_fd(Epoll& epoll);
        bool response_ready() const;
        bool is_removable() const;

        bool io_complete() const;
        Response get_response();
};

//-----------------------------------------------------------------------------
// Common Gateway Interface
//-----------------------------------------------------------------------------
class Cgi
{
    private:
        char **m_envp;
        std::vector<std::shared_ptr<CgiProcess>> m_children;
        const std::chrono::milliseconds m_timeout_ms;
        void reap_dtor(void);
    public:
        Cgi(char **envp, size_t timeout_ms);
        ~Cgi(); // should reap child processes
        Cgi(const Cgi&) = delete;
        Cgi& operator=(const Cgi&) = delete;

        void reap(void);
        void add_process(Client& client, Request& request, Epoll& epoll, const LocationContext* location, const ServerContext& config, Server& server);
        bool is_cgi_fd(int fd) const;
        void timeout(void);
        bool has_children(void) const;
        CgiProcess& get_child(int fd);
        void erase_child(pid_t pid, bool require_connection);
};

#endif