#include "Server.hpp"

#include <iostream>
#include <cstdlib>

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	std::string portStr = argv[1];
	if (!isAllDigits(portStr))
	{
		std::cout << "Error: port must be a number" << std::endl;
		return 1;
	}
	int port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535)
	{
		std::cout << "Error: port must be between 1 and 65535" << std::endl;
		return 1;
	}

	Server server(port, argv[2]);
	server.run();
	return 0;
}
