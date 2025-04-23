#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>

volatile std::sig_atomic_t gSignum;
bool gLive = true;
const int PORT = 8080; //! test port

void signal_handler(int signal)
{
	gSignum = signal;
}

int main()
{
	// Create server socket
	int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (server_fd < 0)
	{
		std::cerr << "Error socket create: " << strerror(errno) << std::endl;
		return 1;
	}

	// Set socket options for reusing address
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		std::cerr << "Setsockopt error: " << strerror(errno) << std::endl;
		return 1;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080); //* converts the unsigned short integer hostshort from host byte order to network byte order

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "bind error: " << strerror(errno) << std::endl;
		close(server_fd);
		return 1;
	}

	// Listen for connections
	if (listen(server_fd, SOMAXCONN) < 0)
	{
		std::cerr << "listen error: " << strerror(errno) << std::endl;
		close(server_fd);
		return 1;
	}

	std::cout << "Server listening on port 8080..." << std::endl;

	// create epoll
	int epoll_fd = epoll_create1(0); //* returns fd referring to new epoll instance
	if (epoll_fd < 0)
	{
		std::cerr << "Epoll creation failed: " << strerror(errno) << std::endl;
		close(server_fd);
		return 1;
	}

	// add server socket to epoll
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0)
	{
		std::cerr << "error adding server socket to epoll: " << strerror(errno) << std::endl;
		close(server_fd);
		close(epoll_fd);
		return 1;
	}

	// event loop
	struct epoll_event events[10]; // random number
	std::cout << "Server started on port " << PORT << "..." << std::endl;

	while (1)
	{
		int num_events = epoll_wait(epoll_fd, events, 10, -1);
		if (num_events < 0)
		{
			std::cerr << "Epoll wait error: " << strerror(errno) << std::endl;
			break;
		}

		// process events
		for (int i = 0; i < num_events; i++)
		{
			// new connection if event on server socket
			if (events[i].data.fd == server_fd)
			{
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);

				int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
				if (client_fd < 0)
				{
					if (errno != EAGAIN && errno != EWOULDBLOCK) // EAGAIN / EWOULDBLOCk = no connection are ready
						std::cerr << "Error accept: " << strerror(errno) << std::endl;
					continue;
				}

				// Add new client socket to epoll's interest list
				// EPOLLIN - when there is data to read
				// EPOLLET - Edge-Triggered mode: only notify when state changes
				event.events = EPOLLIN | EPOLLET;
				event.data.fd = client_fd;

				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0)
				{
					std::cerr << "Error adding client to epoll: " << strerror(errno) << std::endl;
					close(client_fd);
					continue;
				}

				std::cout << "New client connected: " << client_fd << std::endl;
			}

			// else, I/O event on client socket
			else
			{
				int client_fd = events[i].data.fd;

				// check if read event (& bitwise operator -> evaluates binary val of each operand)
				if (events[i].events & EPOLLIN)
				{
					char buff[1024] = {0};
					int bytes_read = recv(client_fd, buff, sizeof(buff) - 1, 0);
					if (bytes_read <= 0)
					{
						if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
							continue;

						std::cout << "Client disconnected: " << client_fd << std::endl;

						// remove socket from epoll interest list
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
						close(client_fd);
					}
					else
					{
						// Null terminate receive data
						buff[bytes_read] = '\0';
						std::cout << "Received from client: " << client_fd << std::endl;

						//* create http response
						//* should parse http request here + generate response based on request content
						const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nContent-Type: text/plain\r\n\r\nHello, World!";
						//! const response is a placeholder!

						// send response back to client
						send(client_fd, response, strlen(response), 0);

						// keep connections open

						//! now closing for the test
						std::cout << "Closing client: " << client_fd << std::endl;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
						close(client_fd);
					}
				}
			}
		}
	}
	close(server_fd);
	close(epoll_fd);
	return 0;
}

//* To test: http://localhost:8080/ in the browser
//* cmd curl http://localhost:8080/ to check url that outputs content, should give same server content
//* Should make a epoll class, webserv class?
