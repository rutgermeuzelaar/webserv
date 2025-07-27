#ifndef CGI_PROCESS_HPP
# define CGI_PROCESS_HPP
# include <string>
# include <chrono>
# include "Server.hpp"

class LocationContext;
class Response;
class ServerContext;
class Epoll;

class CgiProcess
{
    private:
        Server& m_server;
        bool m_client_connected;
        bool m_reaped;
        int m_fd; // will be bidirectional if the request method was POST
        bool m_is_post;
        bool m_in_notify;

        void notify_observer(CgiProcessEvent);
        void check_state(void);

    public:
        CgiProcess(int fd, int client_fd, pid_t pid, const LocationContext* location, const ServerContext& config, Server& server);
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
        void set_is_post(bool is_post);
        void set_read_fd(int fd);
    
        bool get_client_connected(void) const;
        bool get_reaped(void) const;
        int  get_fd(void) const;
        bool get_is_post(void) const;

        void close_pipe_read_end(Epoll& epoll);
        void read_pipe(Epoll& epoll);
        void write_pipe(int fd, void *buffer, size_t count);
        bool response_ready() const;
        bool is_removable() const;

        Response get_response();
};
#endif