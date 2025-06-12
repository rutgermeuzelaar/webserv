#include "Epoll.hpp"

Epoll::Epoll() : m_epoll_fd(-1)
{
	m_epoll_fd = epoll_create(m_max_events);
	if (m_epoll_fd == -1)
		throw EpollException("Failed to create epoll: " + std::string(strerror(errno)));
	m_events.resize(m_max_events);
	std::cout << "Epoll instance created with fd: " << m_epoll_fd << std::endl;
}

Epoll::~Epoll()
{
	close_epoll_instance();
}

void Epoll::addFd(int fd, int events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;

	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw EpollException("Failed to add fd " + std::to_string(fd) + " to epoll: " + std::string(strerror(errno)));
	std::cout << "Added fd " << fd << " to epoll with events: " << events << std::endl;
}

void Epoll::modifyFD(int fd, int events)
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
	std::cout << "Removed fd " << fd << " from epoll" << std::endl;
}

int Epoll::wait()
{
	int num_events = epoll_wait(m_epoll_fd, m_events.data(), m_max_events, m_epoll_timeout);
	if (num_events == -1)
		throw EpollException("epoll_wait failed: " + std::string(strerror(errno)));
	return num_events;
}

void Epoll::close_epoll_instance()
{
	if (m_epoll_fd != -1)
	{
		if (close(m_epoll_fd) == -1)
			std::cerr << "failed to close epollfd error" << std::endl; //TODO use correct exception
		m_epoll_fd == -1;
		std::cout << "Epoll instance closed" << std::endl;
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

bool Epoll::isReadEvent(const epoll_event& event) const
{
	return event.events & EPOLLIN;
}

bool Epoll::isErrorEvent(const epoll_event& event) const
{
	return event.events & EPOLLERR;
}

bool Epoll::isHangupEvent(const epoll_event& event) const
{
	return event.events & EPOLLHUP;
}
