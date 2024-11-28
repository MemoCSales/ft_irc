#include "Server.hpp"
#include "Command.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>  // inet_ntoa
#include <netdb.h>
#include <utility>
#include <stdexcept>
#include <csignal>
#include <fstream>

Server::Server(int& port, const std::string& password) : password(password)
{
	try
	{
		serverFD = socket(AF_INET, SOCK_STREAM, 0);
		if (serverFD <= 0)
		{
			throw std::runtime_error("Can't create socket");
		}
		/**
		 class stores the port and password and uses them during
		 initialization and operation. The welcome message is
		 displayed when the server starts listening on the specified
		 IP address and port. The `SO_REUSEADDR` option is used to
		 allow the server to bind to the port even if it is in the
		 `TIME_WAIT` state. Additionally, signal handlers are set up
		 to handle interruptions and close the server socket
		 gracefully.
		 * 
		 */
		int opt = 1;
		if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
		}

		SockAddressInitializer initializer(port);
		struct sockaddr_in serverAddress = initializer.getAddress();

		if (bind(serverFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		{
			throw std::runtime_error("Can't bind to IP/port");
		}

		if (listen(serverFD, SOMAXCONN) == -1)
		{
			throw std::runtime_error("Can't listen");
		}
		
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &serverAddress.sin_addr, ip, INET_ADDRSTRLEN);
		std::cout << "Server listening on " << ip << ":" << port << "\n";

		setNonBlocking(serverFD);

		struct pollfd serverP_FDs = {serverFD, POLLIN, 0};
		pollFDs.push_back(serverP_FDs);

		pthread_mutex_init(&clientsMutex, NULL);
		pthread_mutex_init(&channelsMutex, NULL);
		setupSignalHandlers();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(1);
	}
}

Server::~Server()
{
	pthread_mutex_destroy(&clientsMutex);
	pthread_mutex_destroy(&channelsMutex);
	close(serverFD);

	for (ClientsIte it = clients.begin(); it != clients.end(); ++it)
	{
		delete it->second;
	}

	for (ChannelIte it = channels.begin(); it != channels.end(); ++it)
	{
		delete it->second;
	}
	removeLockFile();
}

void Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		throw std::runtime_error("Failed to get file descriptor flags: " + std::string(strerror(errno)));
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
	}
}

void Server::handleNewConnection()
{
	try
	{
		struct sockaddr_in clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientFD = accept(serverFD, (struct sockaddr*)&clientAddress, &clientLength);
		if (clientFD < 0)
		{
			throw std::runtime_error("Failed to accept new connection: " + std::string(strerror(errno)));
		}
		setNonBlocking(clientFD);
		struct pollfd pfd = {clientFD, POLLIN, 0};
		pollFDs.push_back(pfd);

		pthread_mutex_lock(&clientsMutex);
		clients.insert(std::make_pair(clientFD, new Client(clientFD)));
		pthread_mutex_unlock(&clientsMutex);

		std::cout << "New client connected: " << clientFD << std::endl;

		ClientHandler* handler = new ClientHandler(this, clientFD);
		pthread_t thread;
		pthread_create(&thread, NULL, ClientHandler::start, handler);
		pthread_detach(thread);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error handling new connection: " << e.what() << std::endl;
	}
}

void Server::handleClient(int clientFD)
{
	char buffer[MAX_BUFFER];
	while (true)
	{
		try
		{
			int nbytes = recv(clientFD, buffer, sizeof(buffer) - 1, 0);
			if (nbytes < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					continue; // No data available, continue the loop
				}
				else
				{
					perror("recv");
					close(clientFD);

					pthread_mutex_lock(&clientsMutex);
					delete clients[clientFD];
					clients.erase(clientFD);
					pthread_mutex_unlock(&clientsMutex);

					break;
				}
			}
			else if (nbytes == 0)
			{
				close(clientFD);

				pthread_mutex_lock(&clientsMutex);
				delete clients[clientFD];
				clients.erase(clientFD);
				pthread_mutex_unlock(&clientsMutex);

				break;
			}
			buffer[nbytes] = '\0';

			pthread_mutex_lock(&clientsMutex);
			ClientsIte it = clients.find(clientFD);
			if (it != clients.end())
			{
				it->second->buffer += buffer;

				// Process commands
				size_t pos;
				while ((pos = it->second->buffer.find("\r\n")) != std::string::npos)
				{
					std::string command = it->second->buffer.substr(0, pos);
					it->second->buffer.erase(0, pos + 2);
					std::cout << "Received command from " << clientFD << ": " << command << std::endl;

					pthread_mutex_lock(&channelsMutex);
					Command::handleCommand(command, it->second, channels);
					pthread_mutex_unlock(&channelsMutex);
				}
			}
			pthread_mutex_unlock(&clientsMutex);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error handling client: " << e.what() << std::endl;
			break;
		}
	}
}

void Server::run()
{
	while (true)
	{
		try
		{
			int poll_count = poll(&pollFDs[0], pollFDs.size(), -1);
			if (poll_count < 0)
			{
				throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
			}

			for (size_t i = 0; i < pollFDs.size(); ++i)
			{
				if (pollFDs[i].revents & POLLIN)
				{
					if (pollFDs[i].fd == serverFD)
						handleNewConnection();
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error in server run loop: " << e.what() << std::endl;
		}
	}
}

void Server::setupSignalHandlers()
{
	signal(SIGINT, Server::signalHandler);
	signal(SIGTERM, Server::signalHandler);
}

void Server::signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received. Closing server socket." << std::endl;
	exit(signum);
}

void Server::createLockFile()
{
	std::ofstream lockFile(lockFilePath.c_str());
	if (!lockFile)
	{
		throw std::runtime_error("Unable to create lock file");
	}
	lockFile.close();
}

void Server::removeLockFile()
{
	std::remove(lockFilePath.c_str());
}