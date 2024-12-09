#include "Server.hpp"
#include "Commands.hpp"
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

Server* Server::instance = NULL;

Server::Server(int& port, const std::string& password) : password(password)
{
	instance = this;
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
		// | SO_REUSEPORT
		//if (bind(listeningSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
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

		char ServerIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &serverAddress.sin_addr, ServerIP, INET_ADDRSTRLEN);
		if (listen(serverFD, SOMAXCONN) == -1)
		{
			std::stringstream ss;

			ss << "Server " << ServerIP << ":" << port << " can't listen" << std::endl;
			throw std::runtime_error(ss.str());
		}
		std::cout << "Server listening on " << ServerIP << ":" << port << "\n";

		setNonBlocking(serverFD);

		struct pollfd serverP_FDs = {serverFD, POLLIN, 0};
		pollFDs.push_back(serverP_FDs);

		pthread_mutex_init(&clientsMutex, NULL);
		pthread_mutex_init(&channelsMutex, NULL);
		setupSignalHandlers();

		// Start the periodic PING task
		// startPingTask();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		exit(1);
	}
}

void Server::handleNewConnection()
{
	try
	{
		struct sockaddr_in clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = accept(serverFD, (struct sockaddr *)&clientAddress, (socklen_t*)&clientLength);
		if (clientSocket < 0)
		{
			throw std::runtime_error("Failed to accept new connection: " + std::string(strerror(errno)));
		}
		setNonBlocking(clientSocket);
		struct pollfd pfd = {clientSocket, POLLIN, 0};
		pollFDs.push_back(pfd);

		pthread_mutex_lock(&clientsMutex);
		Client* newClient = new Client(clientSocket);
		clients.insert(std::make_pair(clientSocket, newClient));

		// Get client's IP address and port
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
		int clientPort = ntohs(clientAddress.sin_port);
		
		std::cout << getColorStr(FGREEN, "New client connected: ") << clientIP << ":" << clientPort
		<< "[" << clientSocket << "]"<< std::endl;

		// Send welcome message
		newClient->sendMessage(welcomeMsg());

		pthread_mutex_unlock(&clientsMutex);

		pthread_t thread;
		pthread_create(&thread, NULL, Server::clientHandler, newClient);
		pthread_detach(thread);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error handling new connection: " << e.what() << std::endl;
	}
}

void Server::handleClient(int clientFD)
{
	pthread_mutex_lock(&clientsMutex);
	std::map<int, Client*>::iterator it = clients.find(clientFD);
	if (it != clients.end())
		it->second->handleRead();
	pthread_mutex_unlock(&clientsMutex);
}

void* Server::clientHandler(void* arg)
{
	Client* client = static_cast<Client*>(arg);
	Server* server = Server::getInstance();
	try
	{
		while (true)
			server->handleClient(client->getFd());
	}
	catch (const std::exception& e) {
		std::cerr << "Error handling client: " << e.what() << std::endl;
		pthread_mutex_unlock(&server->clientsMutex);
		server->removeClient(client->getFd());
	}
	std::cerr << error("END CLIENT", 0);
	return NULL;
}

void Server::run()
{
	while (true)
	{
		try
		{
			int pollCount = poll(pollFDs.data(), pollFDs.size(), -1);
			if (pollCount < 0)
				throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));

			for (size_t i = 0; i < pollFDs.size(); ++i)
			{
				if (pollFDs[i].revents & POLLIN)
				{
					if (pollFDs[i].fd == serverFD)
						handleNewConnection();
					else
						handleClient(pollFDs[i].fd);
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

	// Access the server instance
	Server* server = Server::getInstance();

	// Send a message to each client
	pthread_mutex_lock(&server->clientsMutex);
	for (ClientsIte it = server->clients.begin(); it != server->clients.end(); ++it)
	{
		it->second->sendMessage("Server is shutting down.\n\n");
	}
	pthread_mutex_unlock(&server->clientsMutex);

	// Close the server socket
	close(server->serverFD);

	// Exit the program
	exit(signum);
}

Server::~Server()
{
	pthread_mutex_destroy(&clientsMutex);
	pthread_mutex_destroy(&channelsMutex);
	// pthread_mutex_lock(&clientsMutex);
	for (ClientsIte it = clients.begin(); it != clients.end(); ++it)
	{
		delete it->second;
		close(it->first);
		clients.erase(it);
	}
	// pthread_mutex_unlock(&clientsMutex);
	// pthread_mutex_lock(&channelsMutex);
	for (ChannelIte it = channels.begin(); it != channels.end(); ++it)
	{
		delete it->second;
		// close(it->first);
		channels.erase(it);
	}
	// pthread_mutex_unlock(&channelsMutex);
	// pthread_mutex_destroy(&clientsMutex);
	// pthread_mutex_destroy(&channelsMutex);
	close(serverFD);
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

std::string Server::welcomeMsg()
{
	std::stringstream msg;
	

	msg << "\t⠀⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣄⠀⠀" << std::endl;
	msg << "\t⣠⣾⣿⡟⠛⢻⠛⠛⠛⠛⠛⢿⣿⣿⠟⠛⠛⠛⣿⣿⣷⣄" << std::endl;
	msg << "\t⣿⣿⣿⡇⠀⢸⠀⠀⣿⣿⡇⠀⣿⠁⠀⣠⣤⣤⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⡇⠀⢸⠀⠀⠿⠿⠃⣠⣿⠀⠀⣿⣿⣿⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⡇⠀⢸⠀⠀⣀⣀⠀⠙⣿⠀⠀⣿⣿⣿⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣇⣀⣸⣀⣀⣿⣿⣀⣀⣿⣦⡀⣀⣀⣀⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠿⢿⣿⣿⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣿⣿⣿⡿⠿⠛⠿⡿⠉⠀⠀⠀⠀⠈⠹⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣿⡿⠁⠀⠀⠀⠀⢇⠀⠛⠘⠃⠛⠀⢠⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣿⣧⡀⠛⠘⠃⠛⠀⢑⣤⣄⣀⣤⡀⣿⣿⣿⣿⣿" << std::endl;
	msg << "\t⣿⣿⣿⣿⣿⡗⢀⣀⣀⣀⣤⣾⣿⣿⣿⣿⣷⣾⣿⣿⣿⣿" << std::endl;
	msg << "\t⠙⢿⣿⣿⣿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋" << std::endl;
	msg << "\t⠀⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀" << std::endl;
	msg << "\nWelcome to the FT_IRC server!" << std::endl << std::endl;
	return msg.str();
	// return getColorStr(FGREEN, msg.str());
}

Server* Server::getInstance()
{
	return instance;
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

void Server::removeClient(int clientFD)
{
	pthread_mutex_lock(&clientsMutex);
	ClientsIte it = clients.find(clientFD);
	if (it != clients.end())
	{
		delete it->second;
		close(it->first);
		clients.erase(it);
	}
	pthread_mutex_unlock(&clientsMutex);
	pthread_exit(NULL);
}

std::string const Server::getPassword() const {
	return password;
}

std::map<std::string, Channel*>& Server::getChannels() {
	return channels;
}

std::map<int, Client*>& Server::getClients() {
	return clients;
}

void Server::sendPingToClients() {
	pthread_mutex_lock(&clientsMutex);
	for (ClientsIte it = clients.begin(); it != clients.end(); it++) {
		std::cout << "Sending PING to client: " << it->first << std::endl;
		it->second->sendMessage("PING ping\r\n");
	}
	pthread_mutex_unlock(&clientsMutex);
}

void* pingTask(void* arg) {
	Server* server = static_cast<Server*>(arg);
	while (true) {
		sleep(5);
		server->sendPingToClients();
	}
	return NULL;
}

void Server::startPingTask() {
	pthread_t thread;
	pthread_create(&thread, NULL, pingTask, this);
	pthread_detach(thread);
}
