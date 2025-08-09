#include "Pch.hpp" // IWYU pragma: keep
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>
#include <string>
#include <vector>

#include "Epoll.hpp"
#include "ResponseHandler.hpp"
#include "Defines.hpp"

Epoll::Epoll() : m_epoll_fd(-1)
{
	m_epoll_fd = epoll_create(m_max_events);
	if (m_epoll_fd == -1)
		throw EpollException("Failed to create epoll: " + std::string(strerror(errno)));
	m_events.resize(m_max_events);
	DEBUG("Epoll instance created with fd: " << m_epoll_fd);
}

Epoll::~Epoll()
{
	close_epoll_instance();
}

Epoll::Epoll(const Epoll& src) : m_epoll_fd(-1)
{
	m_epoll_fd = epoll_create(m_max_events);
	if (m_epoll_fd == -1)
		throw EpollException("Failed to create epoll in copy constructor: " + std::string(strerror(errno)));
	m_events = src.m_events;
	DEBUG("Epoll instance created in copy constructor with fd: " << m_epoll_fd);
}

Epoll& Epoll::operator=(const Epoll& src)
{
	if (this != &src)
	{
		close_epoll_instance();
		m_epoll_fd = epoll_create(m_max_events);
		if (m_epoll_fd == -1)
			throw EpollException("Failed to create epoll in assignment operator: " + std::string(strerror(errno)));
		m_events.resize(m_max_events);
		DEBUG("Epoll instance created in assignment operator with fd: " << m_epoll_fd);
	}
	return *this;
}

void Epoll::addFd(int fd, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;

	DEBUG("[epoll_ctl ADD] m_epoll_fd: " << m_epoll_fd << ", fd: " << fd << ", events: " << events);
	int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
	DEBUG("epoll_ctl(ADD) return: " << ret << ", errno: " << errno << " (" << strerror(errno) << ")");
	if (ret == -1)
		throw EpollException("Failed to add fd " + std::to_string(fd) + " to epoll: " + std::string(strerror(errno)));
	DEBUG("Added fd " << fd << " to epoll with events: " << events);
	DEBUG(std::endl);
}

void Epoll::modifyFD(int fd, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;

	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event) == -1)
		throw EpollException("Failed to modify fd " + std::to_string(fd) + " in epoll: " + std::string(strerror(errno)));
}

void Epoll::removeFD(int fd)
{
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1)
		throw EpollException("Failed to delete fd " + std::to_string(fd) + " in epoll: " + std::string(strerror(errno)));
	DEBUG("Removed fd " << fd << " from epoll");
}

int Epoll::wait()
{
	int num_events = epoll_wait(m_epoll_fd, m_events.data(), m_max_events, m_epoll_timeout);
	if (num_events == -1)
    {
        if (errno == EINTR)
        {
            return 0;
        }
        throw EpollException("epoll_wait failed: " + std::string(strerror(errno)));
    }
	return num_events;
}

void Epoll::close_epoll_instance()
{
	if (m_epoll_fd != -1)
	{
		if (close(m_epoll_fd) == -1)
			std::cerr << "failed to close epollfd error" << std::endl; //TODO use correct exception
		m_epoll_fd = -1;
		DEBUG("Epoll instance closed");
	}

}

const std::vector<struct epoll_event>& Epoll::getEvents() const
{
	return m_events;
}

bool Epoll::isServerSocket(int fd, const std::vector<int>& server_sockets) const
{
	for (int server_fd : server_sockets)
		if (fd == server_fd)
			return true;
	return false;
}

bool Epoll::isTypeEvent(const epoll_event& event, int event_type) const
{
	return event.events & event_type;
}

bool Epoll::isTypeEvent(const epoll_event& event, const std::vector<int>& event_types) const
{
    for (const auto& it: event_types)
    {
        if (event.events & it)
        {
            return true;
        }
    }
    return false;
}

void Epoll::notify(int client_fd, ResponseEvent response_event)
{
    switch (response_event)
    {
        case ResponseEvent::MarkEpollOut:
            modifyFD(client_fd, (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP));
            break;
        case ResponseEvent::UnmarkEpollOut:
            modifyFD(client_fd, (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP));
            break;
    }
}
