#include "Pch.hpp"
#include <unistd.h>
#include <string.h>
#include <cassert>
#include "ResponseHandler.hpp"
#include "Server.hpp"

ResponseHandler::ResponseHandler(Server& server, Epoll& epoll)
    : m_server {server}
    , m_epoll {epoll}
{

}

std::vector<Response>::iterator ResponseHandler::find_response(int client_fd)
{
    return (
        std::find_if(
            m_pending_responses.begin(),
            m_pending_responses.end(),
            [client_fd](const Response& entry){
                return entry.getClientFD() == client_fd;
            }
        )
    );
}

void ResponseHandler::add_response(int client_fd, Response& response)
{
    assert(find_response(client_fd) == m_pending_responses.end());
	assert("Headers should be complete here" && response.getHeadersComplete());
    response.setClientFD(client_fd);
    m_pending_responses.emplace_back(response);
    notify_epoll(client_fd, ResponseEvent::MarkEpollOut);
}

void ResponseHandler::add_response(int client_fd, Response&& response)
{
    assert(find_response(client_fd) == m_pending_responses.end());

    response.setClientFD(client_fd);
    m_pending_responses.emplace_back(response);
    notify_epoll(client_fd, ResponseEvent::MarkEpollOut);
}

void ResponseHandler::notify_observer(int client_fd)
{
    m_server.notify_response_sent(client_fd);
}

void ResponseHandler::notify_epoll(int client_fd, ResponseEvent event)
{
    m_epoll.notify(client_fd, event);
}

void ResponseHandler::send_response(int client_fd)
{
    auto it = find_response(client_fd);
    assert(it != m_pending_responses.end());
    size_t bytes_length;
    const std::byte* bytes = it->get_next_bytes(&bytes_length);
	ssize_t bytes_sent = write(client_fd, bytes, bytes_length);
	if (bytes_sent == -1)
    {
        assert("We made a mistake if errno is ENOTSOCK" && errno != ENOTSOCK);
        assert("We made a mistake if errno is EBADF" && errno != EBADF);
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
        remove_response(client_fd);
        return;
    }
    it->increment_bytes_sent(bytes_sent);
    if (static_cast<size_t>(bytes_sent) == bytes_length)
    {
        m_pending_responses.erase(find_response(client_fd));
        notify_epoll(client_fd, ResponseEvent::UnmarkEpollOut);
        notify_observer(client_fd);
    }
}

void ResponseHandler::remove_response(int client_fd)
{
    auto it = find_response(client_fd);
    assert(it != m_pending_responses.end());
    m_pending_responses.erase(it);
    notify_epoll(client_fd, ResponseEvent::UnmarkEpollOut);
}

void ResponseHandler::remove_if_exists(int client_fd)
{
    auto it = find_response(client_fd);
    if (it == m_pending_responses.end())
    {
        return;
    }
    m_pending_responses.erase(it);
    notify_epoll(client_fd, ResponseEvent::UnmarkEpollOut);
}
