#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
//#include <sys/socket.h>
#include <sstream>

void Client::chanelCommands(std::istringstream& stream) {
	std::string subCommand;
	stream >> subCommand;
	if (subCommand == "KICK") {
		std::string channelName, target;
		if (stream >> channelName && stream >> target) {
			this->kickCommand(channelName, target);
		}
	}
	else if (subCommand == "PASS") {
		std::string password;
		if (stream >> password) {
			this->setPass(password);
		} else {
			std::string error = "Invalid PASS command format. Use: PASS <password>\n";
			send(clientSocket, error.c_str(), error.size() + 1, 0);
		}
	}
	else if(subCommand == "EXIT"){
		std::string channelName;
		stream >> channelName;
		this->exitChanel(channelName);
	}
	else if(subCommand == "TOPIC"){
		std::string channelName;
		stream >> channelName;
		std::string topic;
		stream >> topic;
		this->channelTopic(channelName,topic);

	}
	else if (subCommand ==  currentChannel->getName()) {
		std::string message;
		std::getline(stream, message);
		currentChannel->broadcast(message, this);
	}
	else {
		std::string error = "Unknown command.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
	}
}

void	Client::channelTopic(std::string &channelName, std::string &channelTopic)
{// need to check if operator
	if(this->_isOperator)
	{
		for(unsigned long int i = 0; i < channels.size(); i++)
		{
			if(channels[i]->getName() == channelName)
			{
				std::cout << "topic set\n";
				channels[i]->setTopic(channelTopic);
				// currentChannel->setTopic(channelTopic);
			}
				
		}
	}
	else{
		std::cout << "not an operator\n";
	}
	
}

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
		std::istringstream stream(message);
		std::string command;
		stream >> command;

		if (command == "JOIN") {
			std::string channelName;
			stream >> channelName;
			this->joinChannel(channelName);
		}
		else if (currentChannel) {
			std::istringstream stream(message);
			this->chanelCommands(stream);
		}
		else {
			std::string error = "You are not in a channel. Use JOIN #channel to join one.\n";
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
	if (channelName[0] != '#') {
		std::string error = "Channel name must start with '#'.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
		return;
	}

	if (!currentChannel || currentChannel->getName() != channelName) {
		currentChannel = server->getOrCreateChannel(channelName);
		
		if (!currentChannel->getStatus()) {
			currentChannel->setName(channelName);
			currentChannel->setStatus(true);
			currentChannel->setTopic("null");
			std::cout << currentChannel->getName() + " was created.\n";
			this->_isOperator = true;
			currentChannel->addMember(this);
			currentChannel->addOperator(this);
			channels.push_back(currentChannel);
		}
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
				pass.erase(pass.find_last_not_of(" \n\r\t") + 1);

				if(currentChannel->getPassword() == pass)
				{
					this->_isOperator = false;
					currentChannel->addMember(this);
					std::cout <<this << " Joined existing channel: " << currentChannel->getName() << "\n";
					if(currentChannel->getTopic() != "null")
						currentChannel->broadcastTopic(this);
					channels.push_back(currentChannel);

				} else {
					std::string error = "Incorrect password.\n";
					send(clientSocket, error.c_str(), error.size() + 1, 0);
					currentChannel = nullptr;
				}
			}
		}
		else {
			std::cout << this << " Joined existing channel: " << currentChannel->getName() << "\n";
			this->_isOperator = false;
			currentChannel->addMember(this);
			if(currentChannel->getTopic() != "null")
				currentChannel->broadcastTopic(this);
			channels.push_back(currentChannel);

		}
	} else {
		std::cout << "Already in channel: " << channelName << "\n";
	}
}

/// here on kick command for now anyone ho s an operator can kick people from chanels where isn t in
// need to solve it
 void Client::kickCommand(const std::string& channelName, const std::string& targetAddressStr) {
	if (targetAddressStr.empty()) {
		std::string error = "You need to choose a client to kick.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
		return;
	}
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#'.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
		return;
	}

	if (!this->_isOperator) {
		std::string response = "You are not an operator.\n";
		send(clientSocket, response.c_str(), response.size() + 1, 0);
		return;
	}

	Chanel* channel = server->getChannel(channelName);
	if (!channel) {
		std::string error = "Channel does not exist.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
		return;
	}
	//check if the operator who kick s is in that channel
	bool isInChannel = false;
	for(unsigned long int i = 0; i < channels.size(); i++){
		if (channelName == currentChannel->getName())
			isInChannel = true;	
	}
	if(!isInChannel)
	{
		std::string response = "You don t belong to that channel\n";
		send(clientSocket, response.c_str(), response.size() + 1, 0);
		return ;
	}

	void* targetAddress = reinterpret_cast<void*>(std::stoull(targetAddressStr, nullptr, 16));
	bool found = false;

	for (Client* member : channel->getMembers()) {
		if (reinterpret_cast<void*>(member) == targetAddress) {
			if (member->_isOperator) {
				std::string response = "You can't kick an operator.\n";
				send(clientSocket, response.c_str(), response.size() + 1, 0);
			} else {
				channel->removeMember(member);
				std::cout << targetAddressStr << " got kicked from " << channelName << "\n";
			}
			found = true;
			break;
		}
	}

	if (!found) {
		std::string error = "User not found in the channel.\n";
		send(clientSocket, error.c_str(), error.size() + 1, 0);
	}
}

void	Client::setPass(std::string passWord) {
	if (_isOperator) {
		currentChannel->setPassword(passWord);
		currentChannel->setPassFlag(true);
		const std::string message = "Password for chanel set to: " + currentChannel->getPassword() + '\n';
		send(clientSocket, message.c_str(), message.size() + 1, 0);

	} else {
		std::cout << "you are not the operator" << std::endl;
	}
}

void Client::exitChanel(std::string &channelName) {
    bool found = false;
    for (long unsigned int i = 0; i < channels.size(); i++) {
        if (channels[i]->getName() == channelName) {
            channels[i]->removeMember(this);
            found = true;

            if (channels[i]->isEmpty()) {
				// std::cout << "is empty\n";
				channels[i]->setStatus("null");
                channels.erase(channels.begin() + i);
				server->removeChannel(channelName); 
                this->_isOperator = false;
            }
            // if (currentChannel && currentChannel->getName() == channelName) {
            //     currentChannel = nullptr;
            //     this->_isOperator = false;
            // }
            std::string response = "Exited channel.\n";
            send(clientSocket, response.c_str(), response.size() + 1, 0);
            return;
        }
    }
    if (!found) {
        std::string response = "Channel not found.\n";
        send(clientSocket, response.c_str(), response.size() + 1, 0);
    }
}


void	Client::sendInvitation(std::string channelName) { // just a prototype
	std::string response = "You have been invited to join channel: " + channelName + "\n";
	send(clientSocket, response.c_str(), response.size() + 1, 0);

}

void	Client::modeCommands(std::string message) {
	(void)message;
	std::cout <<"entered mode operator something" << std::endl;
}



void Client::setCurrentChannel(Chanel* channel) {
	this->currentChannel = channel;
}

Client::Client() : clientSocket(-1), server(nullptr), currentChannel(nullptr), _isOperator(false) {}

//for removing without id or username

//reinterpret_cast<Client*>
// ex : currentchanel->removeMember(reinterpret_cast<Client*>(0x55b1714a6810));

