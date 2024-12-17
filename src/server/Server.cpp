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

#include <fstream>

Server* Server::instance = NULL;
volatile sig_atomic_t Server::_shutdownFlag = 0;
Server::Server(int& port, const std::string& password) : 
serverFD(-1), password(password)//, _shutdownFlag(0)
{
	instance = this;
	try
	{
		serverFD = socket(AF_INET, SOCK_STREAM, 0);
		if (serverFD <= 0)
		{
			throw std::runtime_error("Can't create socket");
		}
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

		setupSignalHandlers();

		setOperName();
		setOperPassword();

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

		Client* newClient;
		{
			LockGuard lock(clientsMutex);
			newClient = new Client(clientSocket);
			clients.insert(std::make_pair(clientSocket, newClient));

			// Get client's IP address and port
			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
			int clientPort = ntohs(clientAddress.sin_port);

			{
				LockGuard printLock(printMutex);
				std::cout << getColorStr(FGREEN, "New client connected: ") << clientIP << ":" << clientPort
						  << "[" << clientSocket << "]" << std::endl;
			}

			// Send welcome message
			newClient->sendMessage(welcomeMsg());
		}
		if (pthread_create(&newClient->thread, NULL, Server::clientHandler, newClient) != 0)
		{
			LockGuard printLock(printMutex);
			throw std::runtime_error("Failed to create thread for new connection: " + std::string(strerror(errno)));
			LockGuard lock(clientsMutex);
			removeClient(clientSocket);		
			
		}
		if (pthread_detach(newClient->thread) != 0)
		// if (pthread_join(newClient->thread, NULL) != 0)
		{
			LockGuard printLock(printMutex);
			throw std::runtime_error("Failed to detach thread: " + std::string(strerror(errno)));
		}
	}
	catch (const std::exception& e)
	{
		LockGuard printLock(printMutex);
		std::cerr << "Error handling new connection: " << e.what() << std::endl;
	}
}

void Server::handleClient(int clientFD)
{
	// Server* server = Server::getInstance();
	// LockGuard clientlock(server->clientsMutex);
	ClientsIte it = clients.find(clientFD);
	if (it != clients.end())
		it->second->handleRead();
}

void* Server::clientHandler(void* arg)
{
	Client* client = static_cast<Client*>(arg);
	Server* server = Server::getInstance();
	int clientFD = client->getFd();
	try
	{
		while (true)
		{
			{
				// LockGuard lock(server->shutdownMutex);
				// if (server->_shutdownFlag)
				// {
				// 	return NULL;
				// }
			}
			{
				LockGuard clientlock(server->clientsMutex);
				if (server->clients.find(clientFD) == server->clients.end())
				{
					throw std::runtime_error("Client exited: " + std::string(strerror(errno)));
				}
				server->handleClient(clientFD);
			}
		}
	}
	catch (const std::exception& e)
	{
		{
			LockGuard printLock(server->printMutex);
			std::cerr << "Error handling client: " << e.what() << toStr(clientFD) << std::endl;
			std::cerr << error("END CLIENT\r\n", 0);
		}
		{
			LockGuard lock(server->clientsMutex);
			server->removeClient(clientFD);
		}
	}
	{
		LockGuard printLock(server->printMutex);
		std::cerr << error("*END CLIENT\r\n", 0);
	}
	// pthread_exit(NULL);
	return NULL;
}

void Server::run()
{
	Server* server = Server::getInstance();
	while (true)
	{
		try
		{
			{
				LockGuard lock(server->shutdownMutex);
				if (server->_shutdownFlag)
				// if (_shutdownFlag)
					break;
			}
			int pollCount = poll(pollFDs.data(), pollFDs.size(), -1);
			if (pollCount < 0)
				throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));

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
			LockGuard printLock(printMutex);
			std::cerr << "Error in server run loop: " << e.what() << std::endl;
		}
	}
	{
		LockGuard lockClients(clientsMutex);
		for (ClientsIte it = clients.begin(); it != clients.end(); ++it)
		{
			close(it->first);
			delete it->second;
			clients.erase(it);
		}
	}
	{
		LockGuard lock(server->shutdownMutex);
		close(serverFD);
	}
}

void Server::removeClient(int clientFD)
{
	ClientsIte it = clients.find(clientFD);
	if (it != clients.end())
	{
		close(it->first);
		pthread_cancel(it->second->thread);
		delete it->second;
		clients.erase(it);
	}
}

void Server::setupSignalHandlers()
{
	signal(SIGINT, Server::signalHandlerWrapper);
	signal(SIGTERM, Server::signalHandlerWrapper);
}

void Server::signalHandlerWrapper(int signum)
{
	Server* server = Server::getInstance();
	if (server->instance) {
		server->signalHandler(signum);
	}
}

void Server::signalHandler(int signum)
{
	const char* msg = "Interrupt signal received. Shutting down server.\n";
	write(STDERR_FILENO, msg, strlen(msg));
	{
		Server* server = Server::getInstance();
		LockGuard lock(server->shutdownMutex);
		server->_shutdownFlag = 1;
		LockGuard lockClients(server->clientsMutex);
		ClientsIte it = server->clients.begin();
		for (; it != server->clients.end(); ++it)
		{
			const char* shutDownMessage = "Server is shutting down.\n\n";
			send(it->second->getFd(), shutDownMessage, strlen(shutDownMessage), 0);
			close(it->first);
		}
		pthread_cancel(server->pingThread);
	}
	(void)signum;
}


Server::~Server()
{
	LockGuard lock(shutdownMutex);

	for (ChannelIte it = channels.begin(); it != channels.end(); ++it)
	{
		delete it->second;
		channels.erase(it);
	}
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



std::string const Server::getPassword() const {
	return password;
}

std::map<std::string, Channel*>& Server::getChannels() {
	LockGuard lock(channelsMutex);
	return channels;
}

std::map<int, Client*>& Server::getClients() {
	LockGuard lock(clientsMutex);
	return clients;
}

// void Server::sendPingToClients() {
// 	LockGuard lock(clientsMutex);
// 	for (ClientsIte it = clients.begin(); it != clients.end(); it++)
// 	{
// 		{
// 			Server* server = getInstance();
// 			LockGuard lock(server->shutdownMutex);
// 			if (server->_shutdownFlag)
// 			{
// 				break;
// 			}
// 		}
// 		std::cout << "Sending PING to client: " << it->first << std::endl;
// 		it->second->sendMessage("PING ping\r\n");
// 	}

// }

// void*  Server::pingTask(void* arg) {
// 	Server* server = static_cast<Server*>(arg);
// 	while (true) {
// 		sleep(600);
// 		server->sendPingToClients();
// 	}
// 	return NULL;
// }

// void Server::startPingTask() {
// 	pthread_t thread;
// 	pthread_create(&thread, NULL, pingTask, this);
// 	pthread_detach(thread);
// }


void Server::setOperName(void) {
	_operName = OPER_NAME;
}

void Server::setOperPassword(void) {
	_operPassword = OPER_PASS;
}

std::string const Server::getOperName() const {
	return _operName;
}

std::string const Server::getOperPassword() const {
	return _operPassword;
}
