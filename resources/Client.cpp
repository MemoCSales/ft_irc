#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
//#include "Chanel.hpp"

void Client::handleCommunication() {
	char buff[4096];

	while (true) {
		memset(buff, 0, 4096);
		int bytesRecv = recv(clientSocket, buff, 4096, 0);
		if (bytesRecv <= 0) {
			if (bytesRecv == 0) {
				std::cout << "Client disconnected.\n";
			} else {
				std::cerr << "Connection issue.\n";
			}
			break;
		}

		std::string message(buff, bytesRecv);

		if (message.rfind("/join ", 0) == 0) { // Check if message starts with "/join "
			std::string channelName = message.substr(6);
			joinChannel(channelName);
			std::string response = "Joined channel: " + channelName + "\n";
			send(clientSocket, response.c_str(), response.size() + 1, 0);
		}
		else if (message.rfind("/exit",0) == 0) { // Check if message is "/exit"
			exitChanel();
			std::string response = "Exited channel.\n";
			send(clientSocket, response.c_str(), response.size() + 1, 0);
		}
		else if (currentChannel) {
			currentChannel->broadcast(message, this);
		} else {
			std::string error = "You are not in a channel. Use /join <channel_name> to join one.\n";
			send(clientSocket, error.c_str(), error.size() + 1, 0);
		}
	}
	close(clientSocket);
	if (currentChannel) {
		currentChannel->removeMember(this);
	}
	server->removeClient(this);
}

void Client::joinChannel(const std::string& channelName) {
	if (currentChannel) {
		std::string respones = "You need to exit chanel before moving to another.\n";
		send(clientSocket,respones.c_str(), respones.size() + 1, 0);
//		currentChannel->removeMember(this);
	}
	else
	{
		currentChannel = server->getOrCreateChannel(channelName);
		currentChannel->addMember(this);
	}

}

void	Client::exitChanel() {
	if (currentChannel) {
		currentChannel->removeMember(this);
	}
	currentChannel = nullptr;
}


//else if (message == "/exit") { // Check if message is "/exit"
//exitChanel();
//std::string response = "Exited channel.\n";
//send(clientSocket, response.c_str(), response.size() + 1, 0);
//}