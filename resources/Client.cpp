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
		// getting commands
		std::string message(buff, bytesRecv);
		if (currentChannel) {
			this->chanelCommands(message);
		}
		else {
			if (message.rfind("/join ", 0) == 0) {
				const std::string channelName = message.substr(6);
				joinChannel(channelName);
			}
			else
			{
				std::string error = "You are not in a channel. Use /join <channel_name> to join one.\n";
				send(clientSocket, error.c_str(), error.size() + 1, 0);
			}
		}
	}
	close(clientSocket);
	if (currentChannel) {
		currentChannel->removeMember(this);
	}
	server->removeClient(this);
}


void Client::joinChannel(const std::string& channelName) {
	// Check if the channel already exists
	if (!currentChannel || currentChannel->getName() != channelName)
	{
		currentChannel = server->getOrCreateChannel(channelName);
		if (!currentChannel->getStatus()) {
			currentChannel->setName(channelName);
			currentChannel->setStatus(true);
			std::cout << currentChannel->getName() + " was created.\n";
			this->_isOperator = true;
			std::cout << this << " you are the operator" << std::endl;
			currentChannel->addMember(this);
			currentChannel->addOperator(this);
		}
		//check if chanel has a password
		else if(currentChannel && currentChannel->getPassFlag() == true)
		{
			std::string response = "Private channel, require a password\n";
			send(clientSocket, response.c_str(), response.size() + 1, 0);
			// Wait for the client's response
			char passBuff[4096];
			memset(passBuff, 0, 4096);
			int passBytesRecv = recv(clientSocket, passBuff, 4096, 0);
			if (passBytesRecv > 0) {
				std::string pass(passBuff, passBytesRecv);
				if(currentChannel->getPassword() == pass)
				{
					std::cout <<this << " Joined existing channel: " << currentChannel->getName() << "\n";
					this->_isOperator = false;
					currentChannel->addMember(this);
				} else {
					std::string error = "Incorrect password.\n";
					send(clientSocket, error.c_str(), error.size() + 1, 0);
                   currentChannel = nullptr;
				}
			}
		}
		else
		{
			std::cout <<this << " Joined existing channel: " << currentChannel->getName() << "\n";
			this->_isOperator = false;
			currentChannel->addMember(this);

		}
	} else {
		std::cout << "Already in channel: " << channelName << "\n";
	}
}


void	Client::chanelCommands(std::string message)
{
	if (message.rfind("/kick ", 0) == 0) {
		this->kickCommand(message);
	}
	else if (message.rfind("/pass ", 0) == 0) {
		this->setPass(message);
	}
	else if (message.rfind("/exit", 0) == 0) {
		this->exitChanel();
	}
	else {
		currentChannel->broadcast(message, this);
	}
}

void	Client::kickCommand(std::string message) {  // need to check if the operator can kick himself
	if (this->_isOperator) {
		const std::string clientAddressStr = message.substr(6);
		void* clientAddress = reinterpret_cast<void*>(std::stoull(clientAddressStr, nullptr, 16));
		if (reinterpret_cast<Client*>(clientAddress)->_isOperator)
		{
			std::string response = "You can t kick an operator\n";
			send(clientSocket, response.c_str(), response.size() + 1, 0);
		}
		else
		{
			currentChannel->removeMember(reinterpret_cast<Client*>(clientAddress));
			std::cout << clientAddress << " got kicked out" << std::endl;
		}

	} else {
		std::string response = "You are not an operator\n";
		send(clientSocket, response.c_str(), response.size() + 1, 0);
	}
}

void	Client::setPass(std::string message) {
	if (_isOperator) {
		const std::string passWord = message.substr(6);
		currentChannel->setPassword(passWord);
		currentChannel->setPassFlag(true);
		const std::string message = "Password for chanel set to: ";
		send(clientSocket, message.c_str(), message.size() + 1, 0);
		send(clientSocket,currentChannel->getPassword().c_str() , currentChannel->getPassword().size() + 1, 0);

	} else {
		std::cout << "you are not the operator" << std::endl;
	}
}


void	Client::exitChanel() {
	if (currentChannel) {
		currentChannel->removeMember(this);
	}
	currentChannel = nullptr;
	_isOperator = false;
	std::string response = "Exited channel.\n";
	send(clientSocket, response.c_str(), response.size() + 1, 0);
}

void Client::setCurrentChannel(Chanel* channel) {
	this->currentChannel = channel;
}

Client::Client() : clientSocket(-1), server(nullptr), currentChannel(nullptr), _isOperator(false) {}

//for removing without id or username

//reinterpret_cast<Client*>
// ex : currentchanel->removeMember(reinterpret_cast<Client*>(0x55b1714a6810));

