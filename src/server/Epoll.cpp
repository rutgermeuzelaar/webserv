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

Epoll::Epoll(const Epoll& src) : m_epoll_fd(-1)
{
	m_epoll_fd = epoll_create(m_max_events);
	if (m_epoll_fd == -1)
		throw EpollException("Failed to create epoll in copy constructor: " + std::string(strerror(errno)));
	m_events = src.m_events;
	std::cout << "Epoll instance created in copy constructor with fd: " << m_epoll_fd << std::endl;
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
		std::cout << "Epoll instance created in assignment operator with fd: " << m_epoll_fd << std::endl;
	}
	return *this;
}

void Epoll::addFd(int fd, int events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;

   	// std::cout << "fd before epoll_ctl: " << fd << std::endl; //! TEST
    // if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
	std::cout << "[epoll_ctl ADD] m_epoll_fd: " << m_epoll_fd << ", fd: " << fd << ", events: " << events << std::endl; //! DEBUG
	int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
	std::cout << "epoll_ctl(ADD) return: " << ret << ", errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
	if (ret == -1)
		throw EpollException("Failed to add fd " + std::to_string(fd) + " to epoll: " + std::string(strerror(errno)));
	std::cout << "Added fd " << fd << " to epoll with events: " << events << std::endl;
	std::cout << std::endl;
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
	std::cout << "[epoll_wait] m_epoll_fd: " << m_epoll_fd << std::endl; //! DEBUG
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
		m_epoll_fd = -1;
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

bool Epoll::isTypeEvent(const epoll_event& event, int event_type) const
{
	return event.events & event_type;
}
