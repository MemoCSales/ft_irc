#include <iostream>
#include "Client.hpp"
#include "Server.hpp"
#include <string>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	try
	{
		int port;
		std::stringstream ss(argv[1]);
		if (!(ss >> port) || !(ss.eof()) || port <= 0 || port > 65535)
			throw std::invalid_argument("Invalid port number");
		std::string password(argv[2]);

		Server server(port, password);
		server.run();
	} catch (const std::invalid_argument& e)
	{
		std::cerr << "Invalid port number: " << argv[1] << std::endl;
		return 1;
	} catch (const std::out_of_range& e)
	{
		std::cerr << "Port number out of range: " << argv[1] << std::endl;
		return 1;
	}
}