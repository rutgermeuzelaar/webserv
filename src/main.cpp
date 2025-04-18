#include <stdlib.h>
#include <iostream>
#include <sys/epoll.h>

int main(int argc, char **argv)
{
	const int epoll_fd = epoll_create(1);
	struct epoll_event event;
	(void)argv;
	
	if (argc != 2)
	{
		std::cout << "Insufficient arguments, please supply a configuration file.\n";
		return (EXIT_FAILURE);
	}
	if (epoll_fd == -1)
	{
		return (EXIT_FAILURE);
	}
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 5, &event) == -1)
	{
		std::cout << "Hello world!\n";
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}