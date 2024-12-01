
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
	bool _isOperator;


public:
	Client();
	explicit Client(int socket, Server* srv) : clientSocket(socket), server(srv) {}

	void handleCommunication(); // Handles communication with this client
	void joinChannel(const std::string& channelName);
	void exitChanel();
	int getSocket() const { return clientSocket; }

	////just for  testing/debuging ///
	void	setCurrentChannel(Chanel *chanel);
	// commands
	void	chanelCommands(std::string message);
	void	kickCommand(std::string message);
	void 	setPass(std::string message);


};

#endif