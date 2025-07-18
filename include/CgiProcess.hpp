#ifndef CGI_PROCESS_HPP
# define CGI_PROCESS_HPP
# include <string>
# include <chrono>

class LocationContext;
class Response;
class ServerContext;
class Epoll;

class CgiProcess
{
    public:
        CgiProcess(int read_fd, int client_fd, pid_t pid, const LocationContext* location, const ServerContext& config);
        CgiProcess& operator=(const CgiProcess&);

        bool m_reaped;
        int m_read_fd;
        int m_client_fd;
        std::chrono::_V2::steady_clock::time_point m_start;
        pid_t m_pid;
        std::string m_buffer;
        const LocationContext* m_location;
        const ServerContext& m_config;
        bool m_client_connected;
        int m_exit_status;

        void close_pipe_read_end(Epoll& epoll);
        void read_pipe(Epoll& epoll);
        bool response_ready() const;
        Response get_response();
};
#endif