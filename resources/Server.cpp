#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
//#include <stdexcept>
#include <algorithm>

Server::Server(const std::string& ip, uint16_t port) {
	//socket creation
	listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listeningSocket <= 0) {
		throw std::runtime_error("Can't create socket");
	}
	//server address setup
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
	//binds the socket to the specified ip adress and port
	if (bind(listeningSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		throw std::runtime_error("Can't bind to IP/port");
	}
	//listening for conenction
	if (listen(listeningSocket, SOMAXCONN) == -1) {
		throw std::runtime_error("Can't listen");
	}

	std::cout << "Server listening on " << ip << ":" << port << "\n";
}

Server::~Server() {
	close(listeningSocket); // Close listening socket
	for (auto& thread : clientThreads) {
		if (thread.joinable()) thread.join(); // Ensure all threads are joined
	}
}

void Server::start() {
	acceptClients();
}

void Server::acceptClients() {
	while (true) {
		sockaddr_in clientAddr;
		socklen_t clientSize = sizeof(clientAddr);
		int clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &clientSize);

		if (clientSocket == -1) {
			std::cerr << "Problem with client connecting.\n";
			continue; // Skip to the next iteration if accepting fails
		}

		// Resolve client info
		char host[NI_MAXHOST];
		char svc[NI_MAXSERV];
		memset(host, 0, NI_MAXHOST);
		memset(svc, 0, NI_MAXSERV);

		int result = getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
		if (result) {
			std::cout << host << " connected on " << svc << std::endl;
		} else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
			std::cout << host << " connected on port " << ntohs(clientAddr.sin_port) << std::endl;
		}

		addClient(clientSocket);
	}
}

void Server::addClient(int clientSocket) {
	auto client = std::make_unique<Client>(clientSocket, this);
	Client* clientPtr = client.get(); // Save raw pointer for thread function

	{
		std::lock_guard<std::mutex> lock(clientMutex);
		clients.push_back(std::move(client)); // Add to clients vector
	}

	clientThreads.emplace_back(std::thread(&Client::handleCommunication, clientPtr));
}

void Server::removeClient(Client* client) {
	std::lock_guard<std::mutex> lock(clientMutex);
	auto it = std::remove_if(clients.begin(), clients.end(),
							 [client](const std::unique_ptr<Client>& c) {
								 return c.get() == client;
							 });
	clients.erase(it, clients.end());
}

Chanel* Server::getOrCreateChannel(const std::string& name) {
	std::lock_guard<std::mutex> lock(channelMutex);
	if (channels.find(name) == channels.end()) {
		channels[name] = std::make_unique<Chanel>(name);
	}
	return channels[name].get();
}

void Server::removeChannel(const std::string& name) {
	std::lock_guard<std::mutex> lock(channelMutex);
	channels.erase(name);
}

void Server::sendChannelInvitation(const std::string& channelName) {
	std::lock_guard<std::mutex> lock(clientMutex);
	for (const auto& client : clients) {
		client->sendInvitation(channelName);
	}
}