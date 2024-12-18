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

Server* Server::_instance = NULL;
volatile sig_atomic_t Server::_shutdownFlag = 0;

Server::Server(int& port, const std::string& password) : 
	_serverFD(-1), _password(password)
{
	_instance = this;
	try
	{
		_serverFD = socket(AF_INET, SOCK_STREAM, 0);
		if (_serverFD <= 0)
		{
			throw std::runtime_error("Can't create socket");
		}
		int opt = 1;
		if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
		}

		SockAddressInitializer initializer(port);
		struct sockaddr_in serverAddress = initializer.getAddress();

		if (bind(_serverFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		{
			throw std::runtime_error("Can't bind to IP/port");
		}

		char ServerIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &serverAddress.sin_addr, ServerIP, INET_ADDRSTRLEN);
		if (listen(_serverFD, SOMAXCONN) == -1)
		{
			std::stringstream ss;

			ss << "Server " << ServerIP << ":" << port << " can't listen" << std::endl;
			throw std::runtime_error(ss.str());
		}
		std::cout << "Server listening on " << ServerIP << ":" << port << "\n";

		_setNonBlocking(_serverFD);

		struct pollfd serverP_FDs = {_serverFD, POLLIN, 0};
		_pollFDs.push_back(serverP_FDs);

		_setupSignalHandlers();

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

void Server::_handleNewConnection()
{
	try
	{
		struct sockaddr_in clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = accept(_serverFD, (struct sockaddr *)&clientAddress, (socklen_t*)&clientLength);
		if (clientSocket < 0)
		{
			throw std::runtime_error("Failed to accept new connection: " + std::string(strerror(errno)));
		}
		_setNonBlocking(clientSocket);
		struct pollfd pfd = {clientSocket, POLLIN, 0};
		_pollFDs.push_back(pfd);

		Client* newClient;
		{
			LockGuard lock(_clientsMutex);
			newClient = new Client(clientSocket);
			_clients.insert(std::make_pair(clientSocket, newClient));

			// Get client's IP address and port
			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
			int clientPort = ntohs(clientAddress.sin_port);

			{
				LockGuard printLock(_printMutex);
				std::cout << getColorStr(FGREEN, "New client connected: ") << clientIP << ":" << clientPort
						  << "[" << clientSocket << "]" << std::endl;
			}

			// Send welcome message
			newClient->sendMessage(_welcomeMsg());
		}
		if (pthread_create(&newClient->thread, NULL, _clientHandler, newClient) != 0)
		{
			LockGuard printLock(_printMutex);
			throw std::runtime_error("Failed to create thread for new connection: " + std::string(strerror(errno)));
			LockGuard lock(_clientsMutex);
			_removeClient(clientSocket);		
			
		}
		if (pthread_detach(newClient->thread) != 0)
		// if (pthread_join(newClient->thread, NULL) != 0)
		{
			LockGuard printLock(_printMutex);
			throw std::runtime_error("Failed to detach thread: " + std::string(strerror(errno)));
		}
	}
	catch (const std::exception& e)
	{
		LockGuard printLock(_printMutex);
		std::cerr << "Error handling new connection: " << e.what() << std::endl;
	}
}

void Server::_handleClient(int clientFD)
{
	ClientsIte it = _clients.find(clientFD);
	if (it != _clients.end())
		it->second->handleRead();
}

void* Server::_clientHandler(void* arg)
{
	Client* client = static_cast<Client*>(arg);
	Server* server = Server::getInstance();
	int clientFD = client->getFd();
	try
	{
		while (true)
		{
			{
				LockGuard clientlock(server->_clientsMutex);
				if (server->_clients.find(clientFD) == server->_clients.end())
				{
					throw std::runtime_error("Client exited: " + std::string(strerror(errno)));
				}
				server->_handleClient(clientFD);
			}
		}
	}
	catch (const std::exception& e)
	{
		{
			LockGuard printLock(server->_printMutex);
			std::cerr << "Error handling client: " << e.what() << toStr(clientFD) << std::endl;
			std::cerr << error("END CLIENT\r\n", 0);
		}
		{
			LockGuard lock(server->_clientsMutex);
			server->_removeClient(clientFD);
		}
	}
	{
		LockGuard printLock(server->_printMutex);
		std::cerr << error("*END CLIENT\r\n", 0);
	}
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
				LockGuard lock(server->_shutdownMutex);
				if (server->_shutdownFlag)
					break;
			}
			int pollCount = poll(_pollFDs.data(), _pollFDs.size(), -1);
			if (pollCount < 0)
				throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));

			for (size_t i = 0; i < _pollFDs.size(); ++i)
			{
				if (_pollFDs[i].revents & POLLIN)
				{
					if (_pollFDs[i].fd == _serverFD)
						_handleNewConnection();
				}
			}
		}
		catch (const std::exception& e)
		{
			LockGuard printLock(_printMutex);
			std::cerr << "Error in server run loop: " << e.what() << std::endl;
		}
	}
	{
		LockGuard lockClients(_clientsMutex);
		for (ClientsIte it = _clients.begin(); it != _clients.end(); ++it)
		{
			close(it->first);
			delete it->second;
			_clients.erase(it);
		}
	}
	{
		LockGuard lock(server->_shutdownMutex);
		close(_serverFD);
	}
}

void Server::_removeClient(int clientFD)
{
	ClientsIte it = _clients.find(clientFD);
	if (it != _clients.end())
	{
		close(it->first);
		pthread_cancel(it->second->thread);
		delete it->second;
		_clients.erase(it);
	}
}

void Server::_setupSignalHandlers()
{
	signal(SIGINT, Server::_signalHandlerWrapper);
	signal(SIGTERM, Server::_signalHandlerWrapper);
}

void Server::_signalHandlerWrapper(int signum)
{
	Server* server = Server::getInstance();
	if (server->_instance) {
		server->_signalHandler(signum);
	}
}

void Server::_signalHandler(int signum)
{
	const char* msg = "Interrupt signal received. Shutting down server.\n";
	write(STDERR_FILENO, msg, strlen(msg));
	{
		Server* server = Server::getInstance();
		LockGuard lock(server->_shutdownMutex);
		server->_shutdownFlag = 1;
		LockGuard lockClients(server->_clientsMutex);
		ClientsIte it = server->_clients.begin();
		for (; it != server->_clients.end(); ++it)
		{
			const char* shutDownMessage = "Server is shutting down.\n\n";
			send(it->second->getFd(), shutDownMessage, strlen(shutDownMessage), 0);
			close(it->first);
		}
		pthread_cancel(server->_pingThread);
	}
	(void)signum;
}

Server::~Server()
{
	LockGuard lock(_shutdownMutex);

	for (ChannelIte it = _channels.begin(); it != _channels.end(); ++it)
	{
		delete it->second;
		_channels.erase(it);
	}
	close(_serverFD);
	_removeLockFile();
}

void Server::_setNonBlocking(int fd)
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

std::string Server::_welcomeMsg()
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
	return _instance;
}

void Server::_createLockFile()
{
	std::ofstream lockFile(_lockFilePath.c_str());
	if (!lockFile)
	{
		throw std::runtime_error("Unable to create lock file");
	}
	lockFile.close();
}

void Server::_removeLockFile()
{
	std::remove(_lockFilePath.c_str());
}

std::string const Server::getPassword() const {
	return _password;
}

std::map<std::string, Channel*>& Server::getChannels() {
	LockGuard lock(_channelsMutex);
	return _channels;
}

std::map<int, Client*>& Server::getClients() {
	LockGuard lock(_clientsMutex);
	return _clients;
}

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

Mutex& Server::getPrintMutex() {
	return _printMutex;
}
