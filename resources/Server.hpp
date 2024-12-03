#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <netinet/in.h>
#include <map>

#include "Chanel.hpp"
#include "Client.hpp"

class Client;
class Chanel;

class Server {
private:
	int listeningSocket;
	sockaddr_in serverAddr;
	std::vector<std::thread> clientThreads;
	std::vector<std::unique_ptr<Client>> clients;
	std::map<std::string, std::unique_ptr<Chanel>> channels; // Channels map
	std::mutex clientMutex; // Protects the clients vector
	std::mutex channelMutex; // Protects the channels map

	void acceptClients();

public:
	Server(const std::string& ip, uint16_t port);
	~Server();

	void start(); // Starts the server
	void addClient(int clientSocket);
	void removeClient(Client* client);
	// Channel management
	Chanel* getOrCreateChannel(const std::string& name);
	Chanel* getChannel(const std::string& name);
	void removeChannel(const std::string& name);


	void sendChannelInvitation(const std::string& channelName);
};

#endif