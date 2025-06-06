#pragma once

# include <sys/epoll.h>
# include <stdexcept>
# include <vector>
# include <map>
# include "Config.hpp"
# include "Socket.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "RequestHandler.hpp"

class EpollException : public std::runtime_error {
public:
	explicit EpollException(const std::string& message)
		: std::runtime_error("Epoll Error: " + message) {}
};

class Epoll {
private:
	int m_epoll_fd;
	std::vector<struct epoll_event> m_events;
	static const int m_max_events = 1024;
	static const int m_epoll_timeout = -1;
public:
	Epoll();
	~Epoll();

	//* epoll operations
	void addFd(int fd, int events);
	void modifyFD(int fd, int events);
	void removeFD(int fd);
	int wait();
	const std::vector<struct epoll_event>& getEvents() const;
	void close();

	bool isServerSocket(int fd, const std::vector<int>& server_sockets) const;
	bool isReadEvent(const epoll_event& event) const;
	bool isErrorEvent(const epoll_event& event) const;
	bool isHangupEvent(const epoll_event& event) const;
};