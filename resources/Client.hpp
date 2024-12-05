
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
	std::vector<Chanel *> channels;



public:
	Client();
	explicit Client(int socket, Server* srv) : clientSocket(socket), server(srv) {}

	void handleCommunication(); // Handles communication with this client
	void joinChannel(const std::string& channelName);
	void exitChanel(std::string& channelName);
	int getSocket() const { return clientSocket; }

	////just for  testing/debuging ///
	void	setCurrentChannel(Chanel *chanel);
	// commands

	void 	chanelCommands(std::istringstream& stream);
	void 	kickCommand(const std::string& channelName, const std::string& targetName);
	void 	setPass(std::string message);
	void	modeCommands(std::string message);
	void	sendInvitation(std::string channelName);

	void	channelTopic(std::string &channelName, std::string &channelTopic);



};

#endif