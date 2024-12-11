#include "Client.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <unistd.h>

// Client::Client(Server* srv) : server(srv), currentChannel(nullptr) {}

Client::Client(int fd) : _clientFD(fd), _authenticated(false), nickname(""), username(""), _buffer("") {}

void Client::sendMessage(const std::string &message) {
	write(_clientFD, message.c_str(), message.length());
}

Client::~Client() {}

void Client::handleRead() {
	char buffer[MAX_BUFFER];
	int nbytes = recv(_clientFD, buffer, sizeof(buffer) - 1, 0);
	if (nbytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;// No data available
		else
			throw std::runtime_error("Error on recv: " + std::string(strerror(errno)));
	} else if (nbytes == 0) {
		throw std::runtime_error("Client disconnected");
	}
	buffer[nbytes] = '\0';
	this->_buffer += buffer;
	// std::cout << "Message: " << buffer << std::endl;
	// printAsciiDecimal(buffer);
	// std::cout << "Message_: " << _buffer << std::endl;
	// printAsciiDecimal(buffer);

	// Process commands
	Server *server = Server::getInstance();
	CommandParser commandParser(*server);
	size_t pos;
	while ((pos = this->_buffer.find_first_of("\r\n\0")) != std::string::npos) {
		std::string command = this->_buffer.substr(0, pos);
		this->_buffer.erase(0, pos + 1);// check if 2 or 1
		if (!this->_buffer.empty() && this->_buffer[0] == '\n') {
			this->_buffer.erase(0, 1);
		}
		// std::cout << "Received command from " << _clientFD << ": " << command << std::endl;
		// printAsciiDecimal(command);
		// std::cout << "Updated _buffer: " << this->_buffer << std::endl;
		// printAsciiDecimal(this->_buffer);
		// Handle command
		commandParser.parseAndExecute(*this, command, server->getChannels());
	}
}

bool Client::isAuthenticated() const {
	return _authenticated;
}

void Client::setAuthenticated(bool flag) {
	_authenticated = flag;
}

int Client::getFd() const {
	return _clientFD;
}

//-------------my functions--------------

void Client::channelTopic(std::string &channelName, std::string &channelTopic) {
	if (this->_isOperator) {
		for (unsigned long int i = 0; i < channels.size(); i++) {
			if (channels[i]->getName() == channelName) {
				channels[i]->setTopic(channelTopic);
				std::string message = "You set the topic of the channel to: " + channelTopic + "\n";
				send(_clientFD, message.c_str(), message.size() + 1, 0);
			}
		}
	} else {
		std::string message = "You are not an operator\n";//for now
		send(_clientFD, message.c_str(), message.size() + 1, 0);
	}
}

//
//void Client::joinChannel(const std::string &channelName, Server &server) {
//
//	if (channelName[0] != '#') {
//		std::string error = "Channel name must start with '#'.\n";
//		send(_clientFD, error.c_str(), error.size() + 1, 0);
//		return;
//	}
////	Channel *targetChannel = nullptr;
////	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
////		if ((*it)->getName() == channelName) {
////			targetChannel = *it;
////			break;
////		}
////	}
//	if (!currentChannel || currentChannel->getName() != channelName) {
//		currentChannel = server.getOrCreateChannel(channelName);
//		if (!currentChannel->getStatus()) {
//			currentChannel->setName(channelName);
//			currentChannel->setStatus(true);
//			currentChannel->setTopic("null");
//			std::cout << currentChannel->getName() + " was created.\n";
//			this->_isOperator = true;
//			currentChannel->addMember(this);
//			currentChannel->addOperator(this);
//			channels.push_back(currentChannel);
//		} else if (currentChannel && currentChannel->getPassFlag() == true) {
//			std::string response = "Private channel, require a password\n";
//			send(_clientFD, response.c_str(), response.size() + 1, 0);
//			// Wait for the client's response
//			char passBuff[4096];
//			memset(passBuff, 0, sizeof(passBuff));
//			int passBytesRecv = recv(_clientFD, passBuff, sizeof(passBuff) - 1, 0);
//			if (passBytesRecv > 0) {
//				std::string pass(passBuff, passBytesRecv);
//				pass.erase(pass.find_last_not_of(" \n\r\t") + 1);
//				if (currentChannel->getPassword() == pass) {
//					this->_isOperator = false;
//					currentChannel->addMember(this);
//					std::cout << this->nickname << " Joined existing channel: " << currentChannel->getName() << "\n";
//					if (currentChannel->getTopic() != "null") {
//						currentChannel->broadcastTopic(this);
//						channels.push_back(currentChannel);
//					} else {
//						std::string error = "Incorrect password.\n";
//						send(_clientFD, error.c_str(), error.size() + 1, 0);
//						currentChannel = nullptr;
//					}
//				}
//			}
//		} else {
//			std::cout << this << " Joined existing channel: " << currentChannel->getName() << "\n";
//			this->_isOperator = false;
//			currentChannel->addMember(this);
//			if (currentChannel->getTopic() != "null")
//				currentChannel->broadcastTopic(this);
//			channels.push_back(currentChannel);
//		}
//	} else {
//		std::cout << "Already in channel: " << channelName << "\n";
//	}
//}
//
//void Client::kickCommand(const std::string &channelName, const std::string &target, Server &server) {
//	if (target.empty()) {
//		std::string error = "You need to choose a client to kick.\n";
//		send(_clientFD, error.c_str(), error.size() + 1, 0);
//		return;
//	}
//	if (channelName.empty() || channelName[0] != '#') {
//		std::string error = "Channel name must start with '#'.\n";
//		send(_clientFD, error.c_str(), error.size() + 1, 0);
//		return;
//	}
//
//	if (!this->_isOperator) {
//		std::string response = "You are not an operator.\n";
//		send(_clientFD, response.c_str(), response.size() + 1, 0);
//		return;
//	}
//
//	Channel *channel = server.getChannel(channelName);
//	if (!channel) {
//		std::string error = "Channel does not exist.\n";
//		send(_clientFD, error.c_str(), error.size() + 1, 0);
//		return;
//	}
//	//check if the operator who kick s is in that channel
////	bool isInChannel = false;
////	for (unsigned long int i = 0; i < channels.size(); i++) {
////		if (channelName == channel[i].getName())
////			isInChannel = true;
////	}
////	if (!isInChannel) {
////		std::string response = "You don t belong to that channel\n";
////		send(_clientFD, response.c_str(), response.size() + 1, 0);
////		return;
////	}
//	bool found = false;
//	for (Client *member: channel->getMembers()) {
//		if (member->getClientNick() == target) {
//			if (member->_isOperator) {
//				std::string response = "You can't kick an operator.\n";
//				send(_clientFD, response.c_str(), response.size() + 1, 0);
//			}
//			else {
//				channel->removeMember(member);
//				std::cout << target << " got kicked from " << channelName << "\n";
//			}
//			found = true;
//			break;
//		}
//	}
//	if (!found) {
//		std::string error = "User not found in the channel.\n";
//		send(_clientFD, error.c_str(), error.size() + 1, 0);
//	}
//}

void Client::setPass(std::string channelName, std::string passWord) {
	if (this->_isOperator) {
		for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
			if ((*it)->getName() == channelName) {
				(*it)->setPassword(passWord);
				(*it)->setPassFlag(true);
				const std::string message = "Password for channel set to: " + (*it)->getPassword() + '\n';
				send(_clientFD, message.c_str(), message.size() + 1, 0);
				return;
			}
			else
				std::cout << "Channel not found" << std::endl;
		}
	} else {
		std::cout << "You are not the operator" << std::endl;
	}
}

void Client::exitChanel(std::string &channelName, Server &server) {

	bool found = false;
	// Server* server =  Server::getInstance();
	std::cout << "before foor loop\n";
	for (long unsigned int i = 0; i < channels.size(); i++) {
		if (channels[i]->getName() == channelName) {
			channels[i]->removeMember(this);
			found = true;
			std::cout << "before is empty\n";
			if (channels[i]->isEmpty()) {
				std::cout << "is empty\n";
				channels[i]->setStatus("null");
				std::cout << "before erase\n";
				channels.erase(channels.begin() + i);
				std::cout << "before server->remove channel\n";
				server.removeChannel(channelName);
				std::cout << "after server->remove channel\n";
				this->_isOperator = false;
			}
			// if (currentChannel && currentChannel->getName() == channelName) {
			//     currentChannel = nullptr;
			//     this->_isOperator = false;
			// }
			std::string response = "Exited channel.\n";
			send(_clientFD, response.c_str(), response.size() + 1, 0);
			return;
		}
	}
	if (!found) {
		std::string response = "Channel not found.\n";
		send(_clientFD, response.c_str(), response.size() + 1, 0);
	}
}


void Client::sendInvitation(const std::string &channelName, const std::string &targetNick, Server &server) {
	Client *targetClient = server.getClientByNick(targetNick);
	if (targetClient) {
		std::string response = "You have been invited to join channel: " + channelName + "\n";
		targetClient->sendMessage(response);
		// also add him in the "invited vector or allowed vector"
	} else {
		std::string error = "User with nickname " + targetNick + " not found.\n";
		send(_clientFD, error.c_str(), error.size() + 1, 0);
	}
}



void Client::setCurrentChannel(Channel *channel) {
	this->currentChannel = channel;
}

