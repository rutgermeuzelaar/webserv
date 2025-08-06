#include "Pch.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <optional>
#include <cassert>
#include "Cgi.hpp"
#include "Utilities.hpp"
#include "Defines.hpp"
#include "RequestHandler.hpp"
#include "Epoll.hpp"
#include "Server.hpp"
#include "Http.hpp"

CgiProcess::CgiProcess(int fd, int client_fd, pid_t pid, const LocationContext* location, \
    const ServerContext& config, Server& server, HttpBody& http_body)
    : m_server {server}
    , m_client_connected {true}
    , m_reaped {false}
    , m_fd {fd}
    , m_is_post {false}
    , m_in_notify {false}
    , m_http_body {http_body}
    , m_reading_complete {false}
    , m_writing_complete {false}
    , m_client_fd {client_fd}
    , m_start {std::chrono::steady_clock::now()}
    , m_pid {pid}
    , m_location {location}
    , m_config {config}
    , m_exit_status {0}
{

}

CgiProcess& CgiProcess::operator=(const CgiProcess& other)
{
    m_reaped = other.m_reaped;
    m_fd = other.m_fd;
    m_is_post = other.m_is_post;
    m_in_notify = other.m_in_notify;
    m_http_body = other.m_http_body;
    m_reading_complete = other.m_reading_complete;
    m_writing_complete = other.m_writing_complete;
    m_client_fd = other.m_client_fd;
    m_start = other.m_start;
    m_pid = other.m_pid;
    m_location = other.m_location;
    m_client_connected = other.m_client_connected;
    m_exit_status = other.m_exit_status;
    return *this;
}

CgiProcess::~CgiProcess()
{
    std::cout << __func__ << ": " << m_pid << '\n'; 
}

void CgiProcess::close_fd(Epoll& epoll)
{
    if (m_fd == -1)
    {
        return;
    }
    epoll.removeFD(m_fd);
    if (close(m_fd) == -1)
    {
        perror("close");
    }
    set_fd(-1);
}

void CgiProcess::read_fd(Epoll& epoll)
{
    char buffer[RECV_BUFFER_SIZE];
    
    if (m_fd == -1)
    {
        return;
    }
    ssize_t bytes_read = read(m_fd, buffer, RECV_BUFFER_SIZE);
    if (bytes_read == -1)
    {
        perror("read");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (bytes_read == 0)
    {
        m_reading_complete = true;
        if (shutdown(m_fd, SHUT_RD) == -1)
        {
            perror("shutdown");
        }
        if (io_complete())
        {
            close_fd(epoll);
        }
    }
    else
    {
        m_buffer.append(std::string(buffer, bytes_read));
    }
}

bool CgiProcess::response_ready() const
{
    if (m_fd != -1)
    {
        return false;
    }
    if (!m_reaped)
    {
        return false;
    }
    if (!m_client_connected)
    {
        return false;
    }
    return true;
}

bool CgiProcess::is_removable() const
{
    if (m_fd != -1)
    {
        return false;
    }
    if (!m_reaped)
    {
        return false;
    }
    if (m_client_connected)
    {
        return false;
    }
    return true;
}

// CGI scripts are responsible for creating correct HTTP responses
static void parse_cgi_response(std::string& cgi_buffer, Response& response)
{
    assert(!cgi_buffer.empty());
    size_t chars_consumed = 0;
    const auto& headers = parse_http_headers(cgi_buffer, &chars_consumed);

    response.getHeaders().set_headers(headers);
    const std::string& status_header = response.getHeader("Status");
    if (status_header != "")
    {
        response.setStatusLine(static_cast<HTTPStatusCode>(std::atoi(status_header.c_str())));
    }
    assert(response.getHeader("Content-Type") != "");
    response.setBody(cgi_buffer.substr(chars_consumed, cgi_buffer.size() - chars_consumed));
}

Response CgiProcess::get_response()
{
    if (WIFEXITED(m_exit_status) && WEXITSTATUS(m_exit_status) == EXIT_SUCCESS)
    {
        Response response(HTTPStatusCode::OK);
        parse_cgi_response(m_buffer, response);
        return response;
    }
    if (WIFSIGNALED(m_exit_status) && WTERMSIG(m_exit_status) == SIGTERM)
    {
        return build_error_page(
            HTTPStatusCode::GatewayTimeout,
            m_location,
            m_config
        );
    }
    return build_error_page(
        HTTPStatusCode::InternalServerError,
        m_location,
        m_config
    );
}

void CgiProcess::check_state(void)
{
    if (response_ready())
    {
        notify_observer(CgiProcessEvent::ResponseReady);
    }
    else if (is_removable())
    {
        notify_observer(CgiProcessEvent::IsRemovable);
    }
}

void CgiProcess::notify_observer(CgiProcessEvent event)
{
    if (m_in_notify)
    {
        return;   
    }
    m_in_notify = true;
    m_server.notify(*this, event); // self-destructs here
}

void CgiProcess::set_client_connected(bool status)
{
    m_client_connected = status;
    check_state();
}

void CgiProcess::set_reaped(bool status)
{
    m_reaped = status;
    check_state();
}

void CgiProcess::set_fd(int fd)
{
    m_fd = fd;
    check_state();
}

bool CgiProcess::get_client_connected(void) const
{
    return m_client_connected;
}

bool CgiProcess::get_reaped(void) const
{
    return m_reaped;
}

int CgiProcess::get_fd(void) const
{
    return m_fd;
}

void CgiProcess::write_fd(Epoll& epoll)
{
    size_t bytes_count;

    auto bytes = m_http_body.get_next_bytes(&bytes_count);
    const ssize_t bytes_sent = write(m_fd, bytes, bytes_count);
    if (bytes_sent == -1)
    {
        perror("write");
        assert(false);
    }
    m_http_body.increment_bytes_sent(bytes_sent);
    if (m_http_body.fully_sent())
    {
        epoll.notify(m_fd, ResponseEvent::UnmarkEpollOut);
        if (shutdown(m_fd, SHUT_WR) == -1)
        {
            perror("shutdown");
        }
        m_writing_complete = true;
        if (io_complete())
        {
            close_fd(epoll);
        }
    }
}

bool CgiProcess::io_complete(void) const
{
    return (m_reading_complete && m_writing_complete);
}

void CgiProcess::set_is_post(bool status)
{
    m_is_post = status;
}

void CgiProcess::set_reading_complete(bool status)
{
    m_reading_complete = status;
}

void CgiProcess::set_writing_complete(bool status)
{
    m_writing_complete = status;
}
