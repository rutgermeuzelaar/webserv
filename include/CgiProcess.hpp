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
#endif