
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include <string>
#include "Chanel.hpp"

class Server;
class Chanel;
class Client {
private:
	int clientSocket;
	Server* server;
	Chanel* currentChannel; // Pointer to the current channel


public:
	explicit Client(int socket, Server* srv) : clientSocket(socket), server(srv) {}

	void handleCommunication(); // Handles communication with this client
	void joinChannel(const std::string& channelName);
	void exitChanel();
	int getSocket() const { return clientSocket; }
};

#endif