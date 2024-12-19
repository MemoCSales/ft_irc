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
# include "Utils.hpp"

Server* Server::instance = NULL;
Server::Server(int& port, const std::string& password) : password(password),
_serverStatus(0)
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
		char ServerIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &serverAddress.sin_addr, ServerIP, INET_ADDRSTRLEN);
		if (listen(serverFD, SOMAXCONN) == -1)
		{
			std::stringstream ss;

			ss << "Server " << ServerIP << ":" << port << " can't listen" << std::endl;
			ss << "Server " << ServerIP << ":" << port << " can't listen" << std::endl;
			throw std::runtime_error(ss.str());
		}
		std::cout << "Server listening on " << ServerIP << ":" << port << "\n";
		std::cout << "Server listening on " << ServerIP << ":" << port << "\n";

		setNonBlocking(serverFD);

		struct pollfd serverP_FDs = {serverFD, POLLIN, 0};
		pollFDs.push_back(serverP_FDs);

		pthread_mutex_init(&clientsMutex, NULL);
		// pthread_mutex_init(&coutMutex, NULL);

		setupSignalHandlers();

		setOperName();
		setOperPassword();

		// Start the periodic PING task
		startPingTask();
		setOperName();
		setOperPassword();

		// Start the periodic PING task
		startPingTask();
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
		int clientSocket = accept(serverFD, (struct sockaddr *)&clientAddress, (socklen_t*)&clientLength);
		if (clientSocket < 0)
		{
			throw std::runtime_error("Failed to accept new connection: " + std::string(strerror(errno)));
		}
		setNonBlocking(clientSocket);
		struct pollfd pfd = {clientSocket, POLLIN, 0};
		setNonBlocking(clientSocket);
		struct pollfd pfd = {clientSocket, POLLIN, 0};
		pollFDs.push_back(pfd);

		Client* newClient = new Client(clientSocket);
		pthread_mutex_lock(&clientsMutex);
		clients.insert(std::make_pair(clientSocket, newClient));
		pthread_mutex_unlock(&clientsMutex);

		// Get client's IP address and port
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
		int clientPort = ntohs(clientAddress.sin_port);

		std::ostringstream oss;
		oss << "New client connected: " << clientIP << ":" + toStr(clientPort) 
					<< "[" << toStr(clientSocket) << "]";
		Utils::safePrint(oss.str());

		// Send welcome message
		newClient->sendMessage(welcomeMsg());
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error handling new connection: " << e.what() << std::endl;
	}
}

void Server::handleClient(int clientFD)
{
	std::map<int, Client*>::iterator it = clients.find(clientFD);
	if (it != clients.end()) {
		Client* client = it->second;
		try
		{
			client->handleRead();
	std::map<int, Client*>::iterator it = clients.find(clientFD);
	if (it != clients.end()) {
		Client* client = it->second;
		try
		{
			client->handleRead();
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error handling client: " << e.what() << "\r\n";
			removeClient(clientFD);
		}	
		catch(const std::exception& e)
		{
			std::cerr << "Error handling client: " << e.what() << "\r\n";
			removeClient(clientFD);
		}	
	}
}

void Server::run()
{
	while (true)
	{
		if (_serverStatus)
			break;
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
	cleanData();
}

void Server::cleanData()
{
	// Access the server instance
	Server* server = Server::getInstance();

	// Send a message to each client
	for (ClientsIte it = server->clients.begin(); it != server->clients.end(); ++it)
	{
		const char* shutDownMessage = "Server is shutting down.\0";
		send(it->second->getFd(), shutDownMessage, strlen(shutDownMessage), 0);
	}

	pthread_cancel(server->pingThread);
	pthread_join(server->pingThread, NULL);

	// Clear and release memory of pollFDs vector
	server->pollFDs.clear();
	std::vector<pollfd>().swap(server->pollFDs);

	// Delete clients
	for (ClientsIte it = server->clients.begin(); it != server->clients.end(); ++it)
	{
		delete it->second;
		close(it->first);
	}
	server->clients.clear();

	// Delete channels
	for (ChannelIte it = server->channels.begin(); it != server->channels.end(); ++it)
	{
		delete it->second;
	}
	server->channels.clear();

	// Close the server socket
	close(server->serverFD);

	// Exit the program
	exit(1);
}

void Server::setupSignalHandlers()
{
	signal(SIGINT, Server::signalHandler);
	signal(SIGTERM, Server::signalHandler);
}

void Server::signalHandler(int signum)
{
	const char* msg = "Interrupt signal received. Closing server socket.\n";
	write(STDERR_FILENO, msg, strlen(msg));
	Server* server = Server::getInstance();
	server->_serverStatus = signum;
}

Server::~Server()
{
	pthread_mutex_destroy(&clientsMutex);
	// pthread_mutex_destroy(&coutMutex);
	pthread_cancel(pingThread);
	pthread_join(pingThread, NULL);

	pollFDs.clear();
	std::vector<pollfd>().swap(pollFDs);
	
	for (ClientsIte it = clients.begin(); it != clients.end(); ++it) {
		delete it->second;
		close(it->first);
	}
	clients.clear();

	// Delete channels
	for (ChannelIte it = channels.begin(); it != channels.end(); ++it)	{
		delete it->second;
	}
	channels.clear();

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
		close(it->first);
		clients.erase(it);
	}
	pthread_mutex_unlock(&clientsMutex);
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
		Utils::safePrint("Sending PING to client: " + toStr(it->first));
		it->second->sendMessage("PING ping\r\n");
	}
	pthread_mutex_unlock(&clientsMutex);
}

void* pingTask(void* arg) {
	Server* server = static_cast<Server*>(arg);
	while (true) {
		sleep(600);
		server->sendPingToClients();
	}
	return NULL;
}

void Server::startPingTask() {
	pthread_create(&pingThread, NULL, pingTask, this);
	// pthread_detach(pingThread);
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

void Server::sendPingToClients() {
	pthread_mutex_lock(&clientsMutex);
	for (ClientsIte it = clients.begin(); it != clients.end(); it++) {
		Utils::safePrint("Sending PING to client: " + toStr(it->first));
		it->second->sendMessage("PING ping\r\n");
	}
	pthread_mutex_unlock(&clientsMutex);
}

void* pingTask(void* arg) {
	Server* server = static_cast<Server*>(arg);
	while (true) {
		sleep(600);
		server->sendPingToClients();
	}
	return NULL;
}

void Server::startPingTask() {
	pthread_create(&pingThread, NULL, pingTask, this);
	// pthread_detach(pingThread);
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

//------my functions 

Channel* Server::getOrCreateChannel(const std::string& name) {
	// Lock the mutex for thread safety
	// pthread_mutex_lock(&channelsMutex);

	// Check if the channel exists
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end()) {
		// Unlock the mutex before returning
		// pthread_mutex_unlock(&channelsMutex);
		return it->second; // Return the existing channel
	}

	// Create a new channel if not found
	Channel* new_channel = new Channel(name);
	channels[name] = new_channel;
//	new_channel->setName(name);

	// Unlock the mutex before returning
	// pthread_mutex_unlock(&channelsMutex);
	return new_channel; // Return the new channel
}


Channel *Server::getChannel(const std::string &name) {
	// pthread_mutex_lock(&channelsMutex);
	// pthread_mutex_lock(&channelsMutex);
	// Check if the channel exists
	std::map<std::string, Channel *>::iterator it = channels.find(name);
	if (it != channels.end()) {
		// Unlock the mutex before returning
		// pthread_mutex_unlock(&channelsMutex);
		// pthread_mutex_unlock(&channelsMutex);
		return it->second;// Return the existing channel
	}
	// pthread_mutex_unlock(&channelsMutex);
	// pthread_mutex_unlock(&channelsMutex);
	return NULL;// Return the new channel
}

void Server::removeChannel(const std::string& name) {
	// pthread_mutex_lock(&clientsMutex);
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end()) {
		delete it->second;
		channels.erase(it);
	}
	// pthread_mutex_unlock(&clientsMutex);
}

Client* Server::getClientByNick(const std::string& nick) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getClientNick() == nick) {
			return it->second;
		}
	}
	return NULL;
}
