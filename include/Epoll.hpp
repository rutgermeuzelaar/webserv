#pragma once

# include <sys/epoll.h>
# include <stdexcept>
# include <vector>
# include <map>
# include <iostream>
# include <cstring>
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
	static const int m_epoll_timeout = 5;
public:
	Epoll();
	~Epoll();
	Epoll(const Epoll& src);
	Epoll& operator=(const Epoll& src);

	//* epoll operations
	void addFd(int fd, uint32_t events);
	void modifyFD(int fd, int events);
	void removeFD(int fd);
	int wait();
	void close_epoll_instance();
	const std::vector<struct epoll_event>& getEvents() const;

	bool isServerSocket(int fd, const std::vector<int>& server_sockets) const;
	bool isTypeEvent(const epoll_event& event, int event_type) const;
};