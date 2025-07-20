#include "Pch.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <optional>
#include <cassert>
#include "Utilities.hpp"
#include "Response.hpp"
#include "HTTPStatusCode.hpp"
#include "HTTPException.hpp"
#include "Defines.hpp"
#include "CgiProcess.hpp"
#include "RequestHandler.hpp"
#include "Epoll.hpp"
#include "Server.hpp"

CgiProcess::CgiProcess(int read_fd, int client_fd, pid_t pid, const LocationContext* location, const ServerContext& config)
    : m_reaped {false}
    , m_read_fd {read_fd}
    , m_client_fd {client_fd}
    , m_start {std::chrono::steady_clock::now()}
    , m_pid {pid}
    , m_location {location}
    , m_config {config}
    , m_client_connected {true}
{

}

CgiProcess& CgiProcess::operator=(const CgiProcess& cgi_process)
{
    m_read_fd = cgi_process.m_read_fd;
    m_client_fd = cgi_process.m_client_fd;
    m_start = cgi_process.m_start;
    m_pid = cgi_process.m_pid;
    m_client_connected = cgi_process.m_client_connected;
    m_reaped = cgi_process.m_reaped;
    return *this;
}

void CgiProcess::close_pipe_read_end(Epoll& epoll)
{
    if (m_read_fd == -1)
    {
        return;
    }
    epoll.removeFD(m_read_fd);
    if (close(m_read_fd) == -1)
    {
        perror("close");
    }
    set_read_fd(-1);
}

void CgiProcess::read_pipe(Epoll& epoll)
{
    char buffer[RECV_BUFFER_SIZE];
    
    std::cout << __func__ << '\n';
    std::cout << "read(" << m_read_fd << ")\n";
    if (m_read_fd == -1)
    {
        return;
    }
    ssize_t bytes_read = read(m_read_fd, buffer, RECV_BUFFER_SIZE);
    if (bytes_read == -1)
    {
        perror("read");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    // closing read end of pipe
    if (bytes_read == 0)
    {
        std::cout << "close(" << m_read_fd << ")\n";
        close_pipe_read_end(epoll);
    }
    else
    {
        m_buffer.append(std::string(buffer, bytes_read));
    }
}

bool CgiProcess::response_ready() const
{
    if (m_read_fd != -1)
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
    if (m_read_fd != -1)
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

static void parse_cgi_response(std::string& cgi_buffer, Response& response)
{
    const std::string content_type_key = "Content-Type:";

    assert("First line of cgi script should contain the content-type header" && starts_with(cgi_buffer, content_type_key));
    size_t line_break_pos = cgi_buffer.find(LINE_BREAK, content_type_key.size());
    assert(line_break_pos != std::string::npos);
    std::string content_type_value = cgi_buffer.substr(content_type_key.size(), line_break_pos - content_type_key.size());
    content_type_value = trim(content_type_value, WHITE_SPACE);
    response.setHeader("content-type", content_type_value);
    response.setBody(cgi_buffer.substr(line_break_pos + 2, cgi_buffer.size() - line_break_pos + 2));
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

void CgiProcess::set_read_fd(int fd)
{
    m_read_fd = fd;
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

int CgiProcess::get_read_fd(void) const
{
    return m_read_fd;
}