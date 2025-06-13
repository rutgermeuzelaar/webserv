#include <iostream>
#include <csignal>
#include "Server.hpp"
#include "Config.hpp"

volatile std::sig_atomic_t gSignum;
bool gLive = true;

void signal_handler(int signal)
{
    gSignum = signal;
    gLive = false;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Wrong arguments. Usage: ./webserv [config_file]" << std::endl;
        return (EXIT_FAILURE);
    }

    // Set up signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        // Initialize configuration
        Config config;
        read_config_file(config, argv[1]);

        // Create and start server
        Server server(config);
        std::cout << "Starting server..." << std::endl;
        server.start();
		server.run();

        // Cleanup
        std::cout << "Shutting down server..." << std::endl;
        server.stop();
        return (EXIT_SUCCESS);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
}
