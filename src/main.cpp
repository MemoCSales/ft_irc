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
		if (!(ss >> port) || !(ss.eof()))
			throw std::invalid_argument(error("Invalid port number", 0));
		if (port <= 0 || port > 65535)
			throw std::out_of_range(error("Port number out of range:", 0) + toStr(port));
		std::string password(argv[2]);
		
		Server server(port, password);
		server.run();
	} catch (std::invalid_argument const& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	} catch (std::out_of_range const& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}